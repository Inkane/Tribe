
/*
 * Copyright (c) 2010              Dario Freddi <drf@chakra-project.org>
 *               2010, 2011, 2012  Drake Justice <djustice@chakra-project.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <QDebug>
#include <QFile>

#include <unistd.h>

#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>

#include <KIO/Job>
#include <KIO/NetAccess>
#include <KLocalizedString>
#include <KGlobal>

#include <tribepartitionmanager/core/operationstack.h>
#include <tribepartitionmanager/core/partition.h>
#include <tribepartitionmanager/fs/filesystem.h>

#include <config-tribe.h>

#include "pmhandler.h"
#include "installationhandler.h"


class InstallationHandlerHelper
{
public:
    InstallationHandlerHelper()
        : q(0)
    {
    }

    ~InstallationHandlerHelper() {
        delete q;
    }

    InstallationHandler* q;
};

K_GLOBAL_STATIC(InstallationHandlerHelper, s_globalInstallationHandler)

InstallationHandler *InstallationHandler::instance()
{
    if (!s_globalInstallationHandler->q) {
        new InstallationHandler;
    }

    return s_globalInstallationHandler->q;
}

InstallationHandler::InstallationHandler(QObject *parent)
        : QObject(parent),
          m_doc(false),
          m_configurePacman(true)
{
    Q_ASSERT(!s_globalInstallationHandler->q);
    s_globalInstallationHandler->q = this;
}

InstallationHandler::~InstallationHandler()
{
    cleanup();
}

qint64 InstallationHandler::minSizeForTarget() const
{
    return m_minSize;
}

void InstallationHandler::init()
{
    eTime.start();

    QDir dir(INSTALLATION_TARGET);

    if (!dir.exists()) {
        if (!dir.mkpath(INSTALLATION_TARGET)) {
            emit errorInstalling(i18n("Tribe was not able to initialize needed path: ") + QString(INSTALLATION_TARGET) + i18n(" , please resolve the issue, and try again."));
        }
    }

    // compute minimum size for target for partitionpage root part size check
    QFile squash(QString(BOOTMNT_POINT) + "/root-image.sqfs");
    m_minSize = squash.size();
    if (m_minSize <= 0) {
        // Set it to ~700MB
        m_minSize = 700000000;
    }

    m_minSize *= 4;
    m_minSize += 500;
}

void InstallationHandler::setHomeBehaviour(HomeAction act)
{
    homeAction = act;
}

void InstallationHandler::setFileHandlingBehaviour(FileHandling fhnd)
{
    fileAction = fhnd;
}

QStringList InstallationHandler::checkExistingHomeDirs()
{
    QDir dir(QString("%1/home/").arg(INSTALLATION_TARGET));
    return dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::NoSymLinks);
}

int InstallationHandler::antiFlicker()
{
    int retval = eTime.elapsed();

    if (retval > 200) {
        eTime.restart();
    }

    return retval;
}

void InstallationHandler::handleProgress(CurrentAction act, int percentage)
{
    int total;

    if (antiFlicker() < 200) {   // fixme
        return;
    }

    switch (act) {
    case InstallationHandler::DiskPreparation:
        total = (percentage * 10) / 100;
        break;
    case InstallationHandler::SystemInstallation:
        emit streamLabel(i18n("Installing the system ..."));
        total = ( (percentage * 70) / 100 ) + 10;
        break;
    case InstallationHandler::PostInstall:
        total = percentage + 80;
        break;
    default:
        total = 0;
        break;
    }

    emit streamProgress(total);
}

void InstallationHandler::installSystem()
{
    currAction = DiskPreparation;

    // Ok, first, let's see prepare the hard drives through the partitioner
    PMHandler::instance()->apply();
}

void InstallationHandler::partitionsFormatted()
{
    // If we got here, the mount list is already populated. So let's reset the
    // iterator, give back meaningful progress, and start

    resetIterators();
    streamLabel(i18n("Mounting partitions..."));
    mountNextPartition();
}

void InstallationHandler::copyFiles()
{
    currAction = InstallationHandler::SystemInstallation;

    emit streamLabel(i18n("Preparing installation..."));

    // FIXME: why explicitly kill pacman?
    // this should fail instead, and let the user take action
    QProcess::execute("killall pacman");
    QFile::remove("/var/lib/pacman/db.lck");

    QStringList command = QStringList()
            << "unsquashfs"
            << "-f"
            << "-d" << INSTALLATION_TARGET
            << QString("%1/root-image.sqfs").arg(BOOTMNT_POINT);

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, SIGNAL(readyRead()), SLOT(parseUnsquashfsOutput()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(jobDone(int)));

    qDebug() << "Installing (sqfs decompression) started...";

    m_process->start(command.takeFirst(), command);
}

void InstallationHandler::jobDone(int result)
{
    m_process->deleteLater();

    if (result) {
        emit errorInstalling(i18n("Error copying files"));
        return;
    } else {
        emit streamLabel(i18n("Removing packages whose license has been declined ..."));

        postRemove();
    }
}

void InstallationHandler::parseUnsquashfsOutput()
{
    QString out = m_process->readLine(2048);

    if (out.contains(QChar(']'))) {     // todo: make me pretty
        QString parsed = out.split(QChar(']')).at(1);
        QStringList members = parsed.split(QChar('/'));
        QString firstMem = members.at(0);
        firstMem = firstMem.remove(QChar(' '));
        int one = firstMem.toInt();
        QString secondMem = members.at(1);
        secondMem = secondMem.split(QChar(' ')).at(0);
        int two = secondMem.toInt();
        int percentage = (one * 100) / two;
        handleProgress(currAction, percentage);
    }
}

void InstallationHandler::reconnectJobSlot()
{
    if (m_process) {
        connect(m_process, SIGNAL(readyRead()), SLOT(parseUnsquashfsOutput()));
    }
}

void InstallationHandler::readyPost()
{
    populateCommandParameters();

    qDebug() << " :: postinstall command: \n" << m_postcommand;

    m_postjob = "initialize-target";
    m_postlabel = i18n("Initializing target ...");

    postInstall();
}

void InstallationHandler::populateCommandParameters()
{
    qDebug() << " :: System root partition: " << trimDevice(m_mount["/"]);

    m_postcommand = QStringList()
            << "--target-root"    << trimDevice(m_mount["/"])
            << "--target-root-fs" << m_mount["/"]->fileSystem().name()
            << "--mountpoint"     << INSTALLATION_TARGET;

    if (m_mount.contains("swap")) {
        m_postcommand << "--use-swap"    << "yes"
                      << "--target-swap" << trimDevice(m_mount["swap"]);
    }

    if (m_mount.contains("/boot")) {
        m_postcommand << "--use-boot"       << "yes"
                      << "--target-boot"    << trimDevice(m_mount["/boot"])
                      << "--target-boot-fs" << m_mount["/boot"]->fileSystem().name();
    } else {
        m_postcommand << "--use-boot"       << "no"
                      << "--target-boot"    << trimDevice(m_mount["/"]);
    }

    if (m_mount.contains("/home")) {
        m_postcommand << "--use-home" << "yes";
    } else {
        m_postcommand << "--use-home" << "no";
    }

    if (m_mount.contains("/opt")) {
        m_postcommand << "--use-opt" << "yes";
    } else {
        m_postcommand << "--use-opt" << "no";
    }

    if (m_mount.contains("/tmp")) {
        m_postcommand << "--use-tmp" << "yes";
    } else {
        m_postcommand << "--use-tmp" << "no";
    }

    if (m_mount.contains("/usr")) {
        m_postcommand << "--use-usr" << "yes";
    } else {
        m_postcommand << "--use-usr" << "no";
    }

    if (m_mount.contains("/var")) {
        m_postcommand << "--use-var" << "yes";
    } else {
        m_postcommand << "--use-var" << "no";
    }

    if (!m_hostname.isEmpty()) {
        m_postcommand << "--hostname" << m_hostname;
    }

    if (!m_timezone.isEmpty()) {
        m_postcommand << "--timezone" << m_timezone.replace('/', '-');
    }

    if (!m_locale.isEmpty()) {
        m_postcommand << "--locale" << m_locale;
    }

    if (!m_KDELangPack.isEmpty() && m_KDELangPack != "en_us") {
        m_postcommand << "--kdelang" << m_KDELangPack;
    }

    if (m_doc) {
        m_postcommand << "--download-doc" << "yes";
    }

    if (m_configurePacman) {
        m_postcommand << "--configure-pacman" << "yes";
    }
}

QString InstallationHandler::trimDevice(const Partition *device)
{
    QString rdv(device->devicePath());

    return QString("%1%2").arg(rdv.remove(0, 5)).arg(device->number());
}

void InstallationHandler::postRemove()
{
    qDebug() << " :: postinstall package removal: " <<
                m_removeLicenses.count() << "packages to be removed";

    if (m_stringlistIterator == m_removeLicenses.constEnd()) {
        emit streamLabel(i18n("Please wait, configuring the new system ..."));

        QTimer::singleShot(100, this, SLOT(readyPost()));

        currAction = InstallationHandler::PostInstall;

        return;
    }

    QStringList command = QStringList()
            << "sh" << QString("%1/postremove.sh").arg(SCRIPTS_INSTALL_PATH)
            << "--job" << (*m_stringlistIterator)
            << "--mountpoint" << INSTALLATION_TARGET;

    m_process = new QProcess(this);

    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(postRemove()));

    ++m_stringlistIterator;

    qDebug() << " :: package removal command: " << command.join(" ");
    m_process->start(command.takeFirst(), command);
}

void InstallationHandler::postInstall()
{
    if (m_postjob == "add-extra-mountpoint") {
        qDebug() << " :: mountpoints to configure: " << m_mount.keys().count();
        QMap<QString, const Partition*>::const_iterator it;
        for (it = m_mount.constBegin(); it != m_mount.constEnd(); ++it) {
            if (it.key() != "/" && it.key() != "swap") {
                qDebug() << " :: add mountpoint for: " << it.key();
                QStringList command = QStringList()
                        << "sh" << QString("%1/postinstall.sh").arg(SCRIPTS_INSTALL_PATH)
                        << "--job add-extra-mountpoint"
                        << "--extra-mountpoint" << it.key()
                        << "--extra-mountpoint-target" << trimDevice(it.value())
                        << "--extra-mountpoint-fs" << it.value()->fileSystem().name()
                        << m_postcommand;

                qDebug() << " :: postinstall command: " << command.join(" ");

                QProcess process(this);
                process.start(command.takeFirst(), command);
                process.waitForFinished();

                if (process.exitCode() != 0) {
                    postInstallDone(process.exitCode(), QProcess::NormalExit);
                    return;
                }
            }
        }

        postInstallDone(0, QProcess::NormalExit);
    } else {
        QStringList command  = QStringList()
                << "sh" << QString("%1/postinstall.sh").arg(SCRIPTS_INSTALL_PATH)
                << "--job" << m_postjob
                << m_postcommand;

        m_process = new QProcess(this);
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(postInstallDone(int, QProcess::ExitStatus)));

        qDebug() << " :: postinstall command: " << command.join(" ");
        m_process->start(command.takeFirst(), command);
    }
}

void InstallationHandler::postInstallDone(int eC, QProcess::ExitStatus eS)
{
    Q_UNUSED(eS)

    qDebug() << m_process->readAllStandardOutput();
    qDebug() << m_process->readAllStandardError();

    if (eC != 0) {
        emit errorInstalling(i18n("Error in Postinstall script. See log for more details"));
        qDebug() << " :: !! Failed during postinstall somewhere..";
        return;
    } else {
        // next job

        int percentage = 0;

        if (m_postjob == "initialize-target") {
            emit streamLabel(i18n("Creating user accounts ..."));
            setUpUsers(userLoginList());
            m_postjob = "configure-pacman";
            m_postlabel = i18n("Configuring software management...");
            percentage = 1;
        } else if (m_postjob == "configure-pacman") {
            m_postjob = "pre-remove";
            m_postlabel = i18n("Removing unused packages...");
            percentage = 2;
        } else if (m_postjob == "pre-remove") {
            m_postjob = "pre-install";
            m_postlabel = i18n("Installing additional packages...");
            percentage = 3;
        } else if (m_postjob == "pre-install") {
            m_postjob = "setup-xorg";
            m_postlabel = i18n("Configuring the display...");
            percentage = 4;
        } else if (m_postjob == "setup-xorg") {
            m_postjob = "download-l10n";
            m_postlabel = i18n("Downloading and installing localization packages...");
            percentage = 5;
        } else if (m_postjob == "download-l10n") {
            m_postjob = "download-doc";
            m_postlabel = i18n("Downloading and installing documentation packages...");
            percentage = 6;
        } else if (m_postjob == "download-doc") {
            m_postjob = "rcconf-l10n";
            m_postlabel = i18n("Setting up localization...");
            percentage = 7;
        } else if (m_postjob == "rcconf-l10n") {
            m_postjob = "rcconf-daemons";
            m_postlabel = i18n("Creating daemon configuration...");
            percentage = 8;
        } else if (m_postjob == "rcconf-daemons") {
            m_postjob = "rcconf-network";
            m_postlabel = i18n("Creating network configuration...");
            percentage = 9;
        } else if (m_postjob == "rcconf-network") {
            m_postjob = "create-fstab";
            m_postlabel = i18n("Creating fstab...");
            percentage = 10;
        } else if (m_postjob == "create-fstab") {
            m_postjob = "add-extra-mountpoint";
            percentage = 11;
        } else if (m_postjob == "add-extra-mountpoint") {
            m_postjob = "setup-hardware";
            m_postlabel = i18n("Configuring hardware...");
            percentage = 12;
        } else if (m_postjob == "setup-hardware") {
            m_postjob = "create-initrd";
            m_postlabel = i18n("Creating initial ramdisk images...");
            percentage = 13;
        } else if (m_postjob == "create-initrd") {
            m_postjob = "regenerate-locales";
            m_postlabel = i18n("Generating locales...");
            percentage = 16;
        } else if (m_postjob == "regenerate-locales") {
            m_postjob = "cleanup-l10n";
            m_postlabel = i18n("Removing unused localizations...");
            percentage = 17;
        } else if (m_postjob == "cleanup-l10n") {
            m_postjob = "cleanup-drivers";
            m_postlabel = i18n("Removing unused drivers...");
            percentage = 18;
        } else if (m_postjob == "cleanup-drivers") {
            m_postjob = "cleanup-etc";
            m_postlabel = i18n("Finishing up...");
            percentage = 19;
        } else if (m_postjob == "cleanup-etc") {
            m_postjob = "jobcomplete";
            m_postlabel = i18n("Installation complete!");
            percentage = 20;
        }


        m_process->deleteLater();

        handleProgress(InstallationHandler::PostInstall, percentage);
        emit streamLabel(m_postlabel);

        if (m_postjob != "jobcomplete") {
            postInstall();
            return;
        }

        emit installationDone();
    }
}

void InstallationHandler::addPartitionToMountList(const Partition *device, const QString &mountpoint)
{
    m_mount[mountpoint] = device;
}

void InstallationHandler::clearMounts()
{
    m_mount.clear();
}

QString InstallationHandler::getMountPointFor(const QString &device)
{
    QMap<QString, const Partition*>::const_iterator it;

    for (it = m_mount.constBegin(); it != m_mount.constEnd(); ++it) {
        if (QString("%1%2").arg(it.value()->devicePath()).arg(it.value()->number()) == device) {
            return it.key();
        }
    }

    return QString();
}

bool InstallationHandler::isMounted(const QString &partition)
{
    QMap<QString, const Partition*>::const_iterator it;

    for (it = m_mount.constBegin(); it != m_mount.constEnd(); ++it) {
        if (QString("%1%2").arg(it.value()->devicePath()).arg(it.value()->number()) == partition) {
            return it.value()->isMounted();
        }
    }

    return false;
}

QStringList InstallationHandler::getMountedPartitions()
{
    QStringList retlist;

    QMap<QString, const Partition*>::const_iterator it;
    for (it = m_mount.constBegin(); it != m_mount.constEnd(); ++it) {
        if (it.value()->isMounted()) {
            retlist.append(QString("%1%2").arg(it.value()->devicePath()).arg(it.value()->number()));
        }
    }

    return retlist;
}

void InstallationHandler::unmountPartition(const QString &partition)
{
    KIO::Job *job = KIO::unmount(partition, KIO::HideProgressInfo);
    KIO::NetAccess::synchronousRun(job, 0);
}

void InstallationHandler::mountNextPartition()
{
    if (m_mapIterator == m_mount.constEnd()) {
        copyFiles();
        return;
    }

    if (m_mapIterator.key() == "swap") {
        ++m_mapIterator;
        mountNextPartition();
        return;
    }

    QString mntpath = QString("%1%2").arg(INSTALLATION_TARGET).arg(m_mapIterator.key());
    QDir dir(mntpath);

    if (!dir.exists() && !dir.mkpath(mntpath)) {
        emit errorInstalling(i18n("Tribe was not able to initialize needed directories. Something is very wrong..."));
        return;
    }

    QString partitionname = QString("%1%2").arg(m_mapIterator.value()->devicePath()).arg(m_mapIterator.value()->number());

    qDebug() << " :: attempt to mount " << partitionname << " - " << m_mapIterator.key();

    emit mounting(partitionname, InstallationHandler::InProgress);

    KIO::SimpleJob *mJob = KIO::mount(false,
                                      QByteArray(),
                                      partitionname,
                                      QString(INSTALLATION_TARGET + m_mapIterator.key()),
                                      KIO::HideProgressInfo);

    connect(mJob, SIGNAL(result(KJob*)), SLOT(partitionMounted(KJob*)));
}

void InstallationHandler::partitionMounted(KJob *job)
{
    QString partitionname = QString("%1%2").arg(m_mapIterator.value()->devicePath()).arg(m_mapIterator.value()->number());
    if (job->error()) {
        emit errorInstalling(i18n("Could not mount %1", partitionname));
        qDebug() << " :: !! Error mounting: " << job->errorString();

        emit mounting(partitionname, InstallationHandler::Error);

        return;
    } else {
        emit mounting(partitionname, InstallationHandler::Success);

        ++m_mapIterator;

        mountNextPartition();
    }
}

void InstallationHandler::installBootloader(int action, const QString &device)
{
    Q_UNUSED(action);
    Q_UNUSED(device);

    if (m_process) {
        m_process->deleteLater();
    }

    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SIGNAL(bootloaderInstalled(int, QProcess::ExitStatus)));

    QStringList command = QStringList()
            << "sh"
            << QString("%1/postinstall.sh").arg(SCRIPTS_INSTALL_PATH)
            << "--job install-grub2"
            << m_postcommand;

    qDebug() << " :: running bootloader install command:"
             << command.join(" ");

    m_process->start(command.takeFirst(), command);
}

void InstallationHandler::setUpUsers(QStringList users)
{
qDebug() << "::::::: setUpUsers() \n" << users << "\n\n";
    QStringList command;

    int current = 0;

    foreach(QString user, users) {
        if (checkExistingHomeDirs().contains(user)) {
            if (m_userNameList.at(current).isEmpty()) {
                command = QStringList()
                        << "chroot" << INSTALLATION_TARGET
                        << "useradd"
                        << "-d" << QString("/home/%1").arg(user)
                        << "-s" << "/bin/bash"
                        << user;
            } else {
                command = QStringList()
                        << "chroot" << INSTALLATION_TARGET
                        << "useradd"
                        << "-c" << m_userNameList.at(current)
                        << "-d" << QString("/home/%1").arg(user)
                        << "-s" << "/bin/bash"
                        << user;
            }
qDebug() << " :: running useradd command: " << command;
            QProcess::execute(command.takeFirst(), command);
        } else {
            if (m_userNameList.at(current).isEmpty()) {
                command = QStringList()
                        << "chroot" << INSTALLATION_TARGET
                        << "useradd"
                        << "-g" << "users"
                        << "-m"
                        << "-s" << "/bin/bash"
                        << user;
            } else {
                command = QStringList()
                        << "chroot" << INSTALLATION_TARGET
                        << "useradd"
                        << "-g" << "users"
                        << "-c" << m_userNameList.at(current)
                        << "-m"
                        << "-s" << "/bin/bash"
                        << user;
            }
qDebug() << " :: running useradd command: " << command;
            QProcess::execute(command.takeFirst(), command);

            //clean conflict files
            QFile::remove(QString("%1/home/%2/.bash_profile")
                          .arg(INSTALLATION_TARGET)
                          .arg(user));
            QFile::remove(QString("%1/home/%2/.bashrc")
                          .arg(INSTALLATION_TARGET)
                          .arg(user));
            QFile::remove(QString("%1/home/%2/.xinitrc")
                          .arg(INSTALLATION_TARGET)
                          .arg(user));
        }

qDebug() << " :: user \'" + user + "\' created";

        // set kdm/user avatar
        QDir(INSTALLATION_TARGET).mkpath("usr/share/apps/kdm/faces");
        QFile::copy(userAvatarList().at(current),
                    QString("%1/usr/share/apps/kdm/faces/%2.face.icon")
                    .arg(INSTALLATION_TARGET)
                    .arg(user));
        QFile::copy(userAvatarList().at(current),
                    QString("%1/home/%2.face.icon")
                    .arg(INSTALLATION_TARGET)
                    .arg(user));

        // set autologin
        if (m_userAutoLoginList.at(current) == "1") {
            command = QStringList()
                    << "sed"
                    << "-e" << "s,#AutoLoginEnable=true,AutoLoginEnable=true,"
                    << "-e" << QString("s,#AutoLoginUser=fred,AutoLoginUser=%1,").arg(user)
                    << "-i" << QString("%1/usr/share/config/kdm/kdmrc").arg(INSTALLATION_TARGET);
            QProcess::execute(command.takeFirst(), command);
        }

qDebug() << " :: setting user password... : " << m_userLoginList.at(current);
        // set user passwd

        m_passwdCount = current;

        command = QStringList()
                << "chroot" << INSTALLATION_TARGET
                << "/usr/bin/passwd" << user;

        m_userProcess = new QProcess(this);
        m_userProcess->start(command.takeFirst(), command);
        m_userProcess->write(userPasswordList().at(m_passwdCount).toUtf8().constData());
        m_userProcess->write("\n");
        m_userProcess->waitForFinished();

        // copy in live user settings
        QDir dir("/home/live");
        foreach(const QString &ent, dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden)) {
            KIO::Job *job = KIO::copy(KUrl::fromPath(ent),
                                      KUrl::fromPath(QString("%1/home/%2")
                                                     .arg(INSTALLATION_TARGET)
                                                     .arg(user)),
                                      KIO::HideProgressInfo | KIO::Overwrite);

            KIO::NetAccess::synchronousRun(job, 0);
        }

qDebug() << " :: live configuration copied to the user's home";

        command = QStringList()
                << "sh" << QString("%1/postinstall.sh").arg(SCRIPTS_INSTALL_PATH)
                << "--job configure-users" << m_postcommand
                << "--user-name" << user;
        QProcess::execute(command.takeFirst(), command);

qDebug() << ":: user configuration complete";

        command = QStringList()
                << "sh" << QString("%1/postinstall.sh").arg(SCRIPTS_INSTALL_PATH)
                << "--job configure-sudoers" << m_postcommand
                << "--user-name" << user;
        QProcess::execute(command.takeFirst(), command);

qDebug() << " :: sudoers configuration complete";

        current++;
    }

qDebug() << " :: setting root password...";

    // set root passwd

    m_passwdCount = current;

    command = QStringList()
            << "chroot" << INSTALLATION_TARGET
            << "/usr/bin/passwd";

    m_rootUserProcess = new QProcess(this);
    m_rootUserProcess->start(command.takeFirst(), command);
    m_rootUserProcess->write(userPasswordList().last().toAscii().constData());
    m_rootUserProcess->write("\n");
    m_rootUserProcess->waitForFinished();
}

void InstallationHandler::unmountAll()
{
    for (QMap<QString, const Partition*>::const_iterator i = m_mount.constBegin(); i != m_mount.constEnd(); ++i) {
        if (i.key() != "/") {
            QProcess::execute("bash -c \"sudo umount -fl " + QString(INSTALLATION_TARGET + i.key()) + " > /dev/null 2>&1\"");
        }
    }

    foreach (const QString &point, QStringList() << "/proc" << "/sys" << "/dev")
    {
        QProcess::execute("bash -c \"sudo umount -fl " + QString(INSTALLATION_TARGET + point) + " > /dev/null 2>&1\"");
    }

    QProcess::execute("bash -c \"sudo umount -fl " + QString(INSTALLATION_TARGET) + " > /dev/null 2>&1\"");
}

void InstallationHandler::abortInstallation()
{
    cleanup();
}

void InstallationHandler::killProcesses()
{
    if (m_process) {
        m_process->terminate();
        m_process->kill();
    }
}

void InstallationHandler::cleanup()
{
qDebug() << " :: copying installation logs to target /var/log";

    if (QFile::exists("/tmp/installation.log")) {
        QFile::copy("/tmp/installation.log", QString(INSTALLATION_TARGET) + "/var/log/installation.log");
    }

    if (QFile::exists("/tmp/initramfs.log")) {
        QFile::copy("/tmp/initramfs.log", QString(INSTALLATION_TARGET) + "/var/log/installation-initramdisk.log");
    }

    if (QFile::exists("/tmp/grub2.log")) {
        QFile::copy("/tmp/grub2.log", QString(INSTALLATION_TARGET) + "/var/log/installation-grub2.log");
    }

    unmountAll();
    killProcesses();
}

#include "installationhandler.moc"
