/***************************************************************************
 *   Copyright (C) 2008 - 2009 by Dario Freddi                             *
 *   drf@chakra-project.org                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "InstallationHandler.h"

#include <config-tribe.h>

#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <KDebug>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <unistd.h>
#include <klocalizedstring.h>
#include <kglobal.h>
#include <QtCore/QEventLoop>
#include "PMHandler.h"
#include <tribepartitionmanager/core/operationstack.h>
#include <tribepartitionmanager/core/partition.h>
#include <tribepartitionmanager/fs/filesystem.h>
#include <QtCore/QProcess>

class InstallationHandlerHelper
{
public:
    InstallationHandlerHelper() : q(0) {}
    ~InstallationHandlerHelper() {
        delete q;
    }
    InstallationHandler *q;
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

    // Compute minimum size for target
    QFile squash("/.livesys/medium/larch/system.sqf");
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
    hAction = act;
}

void InstallationHandler::setFileHandlingBehaviour(FileHandling fhnd)
{
    fileAct = fhnd;
}

QStringList InstallationHandler::checkExistingHomeDirs()
{
    QDir dir(QString(INSTALLATION_TARGET + QString("home/")));

    if (dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::NoSymLinks).isEmpty())
        return QStringList();
    else
        return dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::NoSymLinks);

    return QStringList();
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

    /* Ok, so let's say how things are handled here:
     *
     *   - System Installation: 80%
     *   - Post Install: 20%
     *
     * This function handles the total progress that has to be shown by the
     * GUI Progressbar. Let's go.
     *
     * We stream stuff with a minimum interval of 0,2 seconds to prevent
     * flickering.
     */

    if (antiFlicker() < 200) {
        /* Don't flicker! */
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

    /* Ok, let's stream a signal with the correct percentage now */

    emit streamProgress(total);
}

void InstallationHandler::installSystem()
{
    currAct = DiskPreparation;

    // Ok, first, let's see prepare the hard drives through the partitioner
    PMHandler::instance()->apply();
}

void InstallationHandler::partitionsFormatted()
{
    // If we got here, the mount list is already populated. So let's reset the iterator, give back meaningful progress, and start
    resetIterators();
    streamLabel(i18n("Mounting partitions..."));
    mountNextPartition();
}

void InstallationHandler::copyFiles()
{
    currAct = InstallationHandler::SystemInstallation;

    emit streamLabel(i18n("Preparing installation, this might take some time ..."));

    /* Let's get what we need to copy */

    QString unsquashfsCommand = "unsquashfs -f -d " + QString(INSTALLATION_TARGET) + " /.livesys/medium/larch/system.sqf";

    kDebug() << "Unsquashfs command is:" << unsquashfsCommand;

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, SIGNAL(readyRead()), SLOT(parseUnsquashfsOutput()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(jobDone(int)));

    kDebug() << "Installing...";

    m_process->start(unsquashfsCommand);

    kDebug() << "Started...";
}

void InstallationHandler::jobDone(int result)
{
    m_process->deleteLater();

    if (result) {
        emit errorInstalling(i18n("Error copying files"));
        kDebug() << "Error installing";
        return;
    } else {
        /* Cool! Let's move on! */

        emit streamLabel(i18n("Removing packages whose license has been declined ..."));

        postRemove();
    }
}

void InstallationHandler::parseUnsquashfsOutput()
{
    QString out = m_process->readLine(2048);

    if (out.contains(QChar(']'))) {
        QString parsed = out.split(QChar(']')).at(1);
        QStringList members = parsed.split(QChar('/'));
        QString firstMem = members.at(0);
        firstMem = firstMem.remove(QChar(' '));
        int one = firstMem.toInt();
        QString secondMem = members.at(1);
        secondMem = secondMem.split(QChar(' ')).at(0);
        int two = secondMem.toInt();
        int percentage = (one * 100) / two;
        handleProgress(currAct, percentage);
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
    qDebug() << "ready for PostInstall";

    populateCommandParameters();

    qDebug() << m_postcommand;

    m_postjob = "initialize-target";
    m_postlabel = i18n("Initializing target ...");

    postInstall();
}

void InstallationHandler::populateCommandParameters()
{
    qDebug() << "m_mount[/] : " << trimDevice(m_mount["/"]);

    m_postcommand = QString("--target-root %1 --target-root-fs %2 --mountpoint %3 ")
                    .arg(trimDevice(m_mount["/"])).arg(m_mount["/"]->fileSystem().name()).arg(INSTALLATION_TARGET);

    if (m_mount.contains("swap")) {
        m_postcommand.append(QString("--use-swap yes --target-swap %1 ").arg(trimDevice(m_mount["swap"])));
    }

    if (m_mount.contains("/boot")) {
        m_postcommand.append(QString("--use-boot yes --target-boot %1 --target-boot-fs %2 ")
                             .arg(trimDevice(m_mount["/boot"])).arg(m_mount["/boot"]->fileSystem().name()));
    } else {
        m_postcommand.append("--use-boot no ");
    }

    if (m_mount.contains("/home")) {
        m_postcommand.append("--use-home yes ");
    } else {
        m_postcommand.append("--use-home no ");
    }

    if (m_mount.contains("/opt")) {
        m_postcommand.append("--use-opt yes ");
    } else {
        m_postcommand.append("--use-opt no ");
    }

    if (m_mount.contains("/tmp")) {
        m_postcommand.append("--use-tmp yes ");
    } else {
        m_postcommand.append("--use-tmp no ");
    }

    if (m_mount.contains("/usr")) {
        m_postcommand.append("--use-usr yes ");
    } else {
        m_postcommand.append("--use-usr no ");
    }

    if (m_mount.contains("/var")) {
        m_postcommand.append("--use-var yes ");
    } else {
        m_postcommand.append("--use-var no ");
    }

    if (!m_hostname.isEmpty()) {
        m_postcommand.append(QString("--hostname %1 ").arg(m_hostname));
    }
    if (!m_timezone.isEmpty()) {
        m_postcommand.append(QString("--timezone %1 ").arg(m_timezone.replace('/', '-')));
    }
    if (!m_locale.isEmpty()) {
        m_postcommand.append(QString("--locale %1 ").arg(m_locale));
    }
    if (!m_KDELangPack.isEmpty() && m_KDELangPack != "en_us") {//American English is just nothing
        m_postcommand.append(QString("--kdelang %1 ").arg(m_KDELangPack));
    }
    if (m_doc) {
        m_postcommand.append("--download-doc yes ");
    }
    if (m_configurePacman) {
        m_postcommand.append("--configure-pacman yes ");
    }
}

QString InstallationHandler::trimDevice(const Partition *device)
{
    QString rdv(device->devicePath());

    return QString("%1%2").arg(rdv.remove(0, 5)).arg(device->number());
}

void InstallationHandler::postRemove()
{
    qDebug() << "Handling postremove..." <<
                m_removeLicenses.count() << "packages have to be removed";

    if (m_stringlistIterator == m_removeLicenses.constEnd()) {
        emit streamLabel(i18n("Please wait, configuring and starting PostInstall ..."));

        QTimer::singleShot(100, this, SLOT(readyPost()));

        currAct = InstallationHandler::PostInstall;

        return;
    }

    QString command  = QString("sh %1 postremove.sh --job %2 --mountpoint %3").arg(SCRIPTS_INSTALL_PATH)
                       .arg(*m_stringlistIterator).arg(INSTALLATION_TARGET);

    m_process = new QProcess(this);

    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(postRemove()));

    ++m_stringlistIterator;

    m_process->start(command);

    qDebug() << "Process " << command << " Started";
}

void InstallationHandler::postInstall()
{
    if (m_postjob == "add-extra-mountpoint") {
        qDebug() << "here we are, we have" << m_mount.keys().count() << "mountpoints to configure";
        QMap<QString, const Partition*>::const_iterator i;
        for (i = m_mount.constBegin(); i != m_mount.constEnd(); ++i) {
            if (i.key() != "/" && i.key() != "swap") {
                qDebug() << "Add mountpoint for" << i.key();
                QString command = QString("sh ") + SCRIPTS_INSTALL_PATH +
                                  QString("postinstall.sh --job add-extra-mountpoint --extra-mountpoint %1 "
                                          "--extra-mountpoint-target %2 --extra-mountpoint-fs %3 %4").arg(i.key())
                                          .arg(trimDevice(i.value())).arg(i.value()->fileSystem().name())
                                          .arg(m_postcommand);

                QProcess *process = new QProcess(this);
                QEventLoop e;
                connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), &e, SLOT(quit()));
                process->start(command);
                e.exec();

                qDebug() << "Process " << command << " Started" << process->exitCode();
                if (process->exitCode() != 0) {
                    postInstallDone(process->exitCode(), QProcess::NormalExit);
                    return;
                }
                process->deleteLater();
            }
        }
	
        postInstallDone(0, QProcess::NormalExit);
    } else {
        QString command  = QString("sh ") +
                           SCRIPTS_INSTALL_PATH +
                           QString("postinstall.sh --job %1 %2")
                           .arg(m_postjob).arg(m_postcommand);

        m_process = new QProcess(this);
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(postInstallDone(int, QProcess::ExitStatus)));
        m_process->start(command);

        qDebug() << "Process " << command << " Started";
    }
}

void InstallationHandler::postInstallDone(int eC, QProcess::ExitStatus eS)
{
    Q_UNUSED(eS)

    qDebug() << m_process->readAllStandardOutput();
    qDebug() << m_process->readAllStandardError();

    if (eC != 0) {
        /* Something went wrong, stream a more verbose message here. */
        emit errorInstalling(i18n("Error in Postinstall script. See log for more details"));
        qDebug() << "Failed";
        return;
    } else {
        /* Kewl, now we need to jump over to the next job.
         */

        int percentage = 0;

        if (m_postjob == "initialize-target") {
            emit streamLabel(i18n("Creating users ..."));
            setUpUser(userLogin());
            m_postjob = "configure-pacman";
            m_postlabel = i18n("Detecting best mirror server & configuring pacman, this may take a bit...");
            percentage = 1;
        } else if (m_postjob == "configure-pacman") {
            m_postjob = "pre-remove";
            m_postlabel = i18n("Removing superfluous packages...");
            percentage = 2;
        } else if (m_postjob == "pre-remove") {
            m_postjob = "pre-install";
            m_postlabel = i18n("Installing additional packages...");
            percentage = 3;
        } else if (m_postjob == "pre-install") {
            m_postjob = "setup-xorg";
            m_postlabel = i18n("Setting up X.Org configuration...");
            percentage = 4;
        } else if (m_postjob == "setup-xorg") {
            m_postjob = "download-l10n";
            m_postlabel = i18n("Downloading and installing localization packages, this may take a bit...");
            percentage = 5;
        } else if (m_postjob == "download-l10n") {
            m_postjob = "download-doc";
            m_postlabel = i18n("Downloading and installing documentation packages, this may take a bit...");
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
            // Just keep the same label, the user does not care, we're still mounting
            percentage = 11;
        } else if (m_postjob == "add-extra-mountpoint") {
            m_postjob = "setup-hardware";
            m_postlabel = i18n("Configuring hardware...");
            percentage = 12;
        } else if (m_postjob == "setup-hardware") {
            m_postjob = "create-initrd";
            m_postlabel = i18n("Creating initial ramdisk images. This may take a lot of time, stay tuned...");
            percentage = 13;
        } else if (m_postjob == "create-initrd") {
            m_postjob = "regenerate-locales";
            m_postlabel = i18n("Regenerating locales...");
            percentage = 16;
        } else if (m_postjob == "regenerate-locales") {
            m_postjob = "cleanup-l10n";
            m_postlabel = i18n("Removing superfluous localizations...");
            percentage = 17;
        } else if (m_postjob == "cleanup-l10n") {
            m_postjob = "cleanup-drivers";
            m_postlabel = i18n("Removing superfluous drivers...");
            percentage = 18;
        } else if (m_postjob == "cleanup-drivers") {
            m_postjob = "cleanup-etc";
            m_postlabel = i18n("Finishing installation...");
            percentage = 19;
        } else if (m_postjob == "cleanup-etc") {
            m_postjob = "jobcomplete";
            m_postlabel = i18n("Installation complete!");
            percentage = 20;
        }


        m_process->deleteLater();

        /* At this point, let's stream updated percentage now. */

        handleProgress(InstallationHandler::PostInstall, percentage);
        emit streamLabel(m_postlabel);

        if (m_postjob != "jobcomplete") {
            /* Ok, we have something to do, so let's just do it! */

            postInstall();

            return;
        } else {
            /* We're done!! Party on and notify the whole application
             * we're having a beer in short time :D
             */

            emit installationDone();
        }
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
    QMap<QString, const Partition*>::const_iterator i;
    for (i = m_mount.constBegin(); i != m_mount.constEnd(); ++i) {
        if (QString("%1%2").arg(i.value()->devicePath()).arg(i.value()->number()) == device) {
            return i.key();
        }
    }

    return QString();
}

bool InstallationHandler::isMounted(const QString &partition)
{
    kDebug() << "Check if" << partition << "is mounted";

    QMap<QString, const Partition*>::const_iterator i;
    for (i = m_mount.constBegin(); i != m_mount.constEnd(); ++i) {
        if (QString("%1%2").arg(i.value()->devicePath()).arg(i.value()->number()) == partition) {
            return i.value()->isMounted();
        }
    }

    return false;
}

QStringList InstallationHandler::getMountedPartitions()
{
    QStringList retlist;

    QMap<QString, const Partition*>::const_iterator i;
    for (i = m_mount.constBegin(); i != m_mount.constEnd(); ++i) {
        if (i.value()->isMounted()) {
            retlist.append(QString("%1%2").arg(i.value()->devicePath()).arg(i.value()->number()));
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
    kDebug() << "Mounting partitions";

    if (m_mapIterator == m_mount.constEnd()) {
        // Start copying files
        copyFiles();
        return;
    }

    if (m_mapIterator.key() == "swap") {
        ++m_mapIterator;
        mountNextPartition();
        return;
    }

    QDir dir(QString(INSTALLATION_TARGET + m_mapIterator.key()));

    if (!dir.exists()) {
        if (!dir.mkpath(QString(INSTALLATION_TARGET + m_mapIterator.key()))) {
            emit errorInstalling(i18n("Tribe was not able to initialize needed directories. You probably do not have "
                                      "enough privileges, please restart Tribe as root."));
            return;
        }
    }

    QString partitionname = QString("%1%2").arg(m_mapIterator.value()->devicePath()).arg(m_mapIterator.value()->number());

    kDebug() << "Trying to mount" << partitionname << "on" << m_mapIterator.key();

    emit mounting(partitionname, InstallationHandler::InProgress);

    KIO::SimpleJob *mJob = KIO::mount(false,
                                      QByteArray(),
                                      partitionname,
                                      QString(INSTALLATION_TARGET + m_mapIterator.key()),
                                      KIO::HideProgressInfo
                                      );

    connect(mJob, SIGNAL(result(KJob*)), SLOT(partitionMounted(KJob*)));
}

void InstallationHandler::partitionMounted(KJob *job)
{
    QString partitionname = QString("%1%2").arg(m_mapIterator.value()->devicePath()).arg(m_mapIterator.value()->number());
    if (job->error()) {
        emit errorInstalling(i18n("Could not mount %1", partitionname));
        kDebug() << "Error with mounting" << job->errorString();
        kDebug() << "error installing";

        emit mounting(partitionname, InstallationHandler::Error);

        return;
    } else {
        /* Cool! Let's move on! */
        emit mounting(partitionname, InstallationHandler::Success);

        ++m_mapIterator;

        mountNextPartition();
    }
}

void InstallationHandler::installBootloader(int action, const QString &device)
{
    if (m_process)
        m_process->deleteLater();

    QString command;

    if (action == 0) {
        command = QString("sh ") + SCRIPTS_INSTALL_PATH + QString("postinstall.sh --job create-menulst %1")
                  .arg(m_postcommand);
    } else {
        command = QString("sh ") + SCRIPTS_INSTALL_PATH + QString("postinstall.sh --job install-grub %1")
                  .arg(m_postcommand);
    }

    QString partition = trimDevice(m_mount["/"]);

    partition.remove(0, 3);

    int grubpart = partition.toInt() - 1;

    command.append(QString("--grub-device %1 --grub-partition %2 ").arg(device).arg(grubpart));

    m_process = new QProcess(this);

    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SIGNAL(bootloaderInstalled(int, QProcess::ExitStatus)));

    m_process->start(command);

    kDebug() << "Process " << command << " Started";
}

void InstallationHandler::setUpUser(const QString &user)
{
    QString command;

    if (checkExistingHomeDirs().contains(userLogin())) {
        command = QString("chroot %1 useradd -c '%2' -d /home/%3 -s /bin/bash %3")
        .arg(INSTALLATION_TARGET)
        .arg(userName())
        .arg(userLogin());
	QProcess::execute(command);
    } else {
        command = QString("chroot %1 useradd -g users -m -s /bin/bash %2")
        .arg(INSTALLATION_TARGET).arg(userLogin());
	QProcess::execute(command);
	//clean conflict files
	command = QString("chroot %1 rm -v /home/%2/.bash_profile")
        .arg(INSTALLATION_TARGET).arg(userLogin());
	QProcess::execute(command);
	command = QString("chroot %1 rm -v /home/%2/.bashrc")
        .arg(INSTALLATION_TARGET).arg(userLogin());
	QProcess::execute(command);
	command = QString("chroot %1 rm -v /home/%2/.xinitrc")
        .arg(INSTALLATION_TARGET).arg(userLogin());
	QProcess::execute(command);
    }

    qDebug() << "user " + userLogin() + " creation completed";
    
    m_userProcess = new QProcess;

    command = QString("chroot %1 /usr/bin/passwd %2").arg(INSTALLATION_TARGET).arg(userLogin());
    connect(m_userProcess, SIGNAL(readyReadStandardError()), SLOT(streamPassword()));
    m_userProcess->start(command);

    QDir dir("/home/live");
    foreach(const QString &ent, dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden)) {
        KIO::Job *job = KIO::copy(KUrl::fromPath(ent),
                                  KUrl::fromPath(QString("%1/home/%2").arg(INSTALLATION_TARGET)
                                  .arg(userLogin())),
             KIO::HideProgressInfo | KIO::Overwrite);

        KIO::NetAccess::synchronousRun(job, 0);
    }

    qDebug() << "/home/live/{config} copied to /home/" + userLogin() + "/{config}";

    QProcess::execute("sh " +
                      QString(SCRIPTS_INSTALL_PATH) +
                      "postinstall.sh --job configure-users " +
                      m_postcommand +
                      " --user-name " +
                      user);

    qDebug() << "job-configure-users finished";

    QProcess::execute("sh " +
                      QString(SCRIPTS_INSTALL_PATH) +
                      "postinstall.sh --job configure-sudoers " +
                      m_postcommand +
                      " --user-name " +
                      user);

    qDebug() << "job-configure-sudoers finished";

    command = "chroot " + QString(INSTALLATION_TARGET) + " /usr/bin/passwd";

    connect(m_userProcess, SIGNAL(readyReadStandardError()), SLOT(streamRootPassword()));
    m_userProcess->start(command);
}

void InstallationHandler::streamRootPassword()
{
    m_userProcess->write(QString(rootPassword()).toUtf8().data());
    m_userProcess->write("\n");
}

void InstallationHandler::streamPassword()
{
    m_userProcess->write(QString(userPassword()).toUtf8().data());
    m_userProcess->write("\n");
}

void InstallationHandler::unmountAll()
{
    for (QMap<QString, const Partition*>::const_iterator i = m_mount.constBegin(); i != m_mount.constEnd(); ++i) {
        if (i.key() != "/") {
            QProcess::execute("sudo umount -fl " + QString(INSTALLATION_TARGET + i.key()));
        }
    }

    // Ok, done that, unmount the nodes.
    foreach (const QString &point, QStringList() << "/proc" << "/sys" << "/dev")
    {
        QProcess::execute("sudo umount -fl " + QString(INSTALLATION_TARGET + point));
    }

    // Now unmount the target itself
    {
        QProcess::execute("sudo umount -fl " + QString(INSTALLATION_TARGET));
    }
}

void InstallationHandler::abortInstallation()
{
    disconnect(m_process, SIGNAL(readyRead()), this, SLOT(parseUnsquashfsOutput()));
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(jobDone(int)));
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(postRemove()));
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(postInstallDone(int, QProcess::ExitStatus)));
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
               SIGNAL(bootloaderInstalled(int,QProcess::ExitStatus)));
    killProcesses();
    cleanup();
    killProcesses();
}

void InstallationHandler::killProcesses()
{
    if (m_process) {
        m_process->terminate();
        m_process->kill();
        m_process->deleteLater();
    }
}

void InstallationHandler::cleanup()
{
    kDebug() << "Cleaning up...";
    unmountAll();
    killProcesses();
}

#include "InstallationHandler.moc"
