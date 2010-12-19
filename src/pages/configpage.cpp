/*
 * Copyright (c) 2008, 2009  Dario Freddi <drf@chakra-project.org>
 *               2010        Drake Justice <djustice.kde@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <QDebug>

#include <QFile>
#include <QMovie>
#include <QProcess>
#include <QRegExpValidator>

#include <KIcon>
#include <KIO/Job>

#include <config-tribe.h>

#include "../installationhandler.h"
#include "configpage.h"


const QString tI = "/tmp/tribe_initcpio_enable_";
QStringList tmpInitRd = QStringList() << QString(tI + "usb") << QString(tI + "firewire")
                                      << QString(tI + "pcmcia") << QString(tI + "nfs")
                                      << QString(tI + "softwareraid") << QString(tI + "softwareraidmdp")
                                      << QString(tI + "lvm2") << QString(tI + "encrypted");


ConfigPage::ConfigPage(QWidget *parent)
        : AbstractPage(parent),
        m_install(InstallationHandler::instance())
{
    m_process = new QProcess(this);
}

ConfigPage::~ConfigPage()
{
}

void ConfigPage::createWidget()
{
    ui.setupUi(this);
    
    ui.changeAppearanceButton->setVisible(false);

    // page connections
    connect(ui.installPkgzButton, SIGNAL(clicked()), this, SLOT(setInstallPkgzPage()));
    connect(ui.downloadBundlesButton, SIGNAL(clicked()), this, SLOT(setDownloadBundlesPage()));
    connect(ui.changeAppearanceButton, SIGNAL(clicked()), this, SLOT(setChangeAppearancePage()));
    connect(ui.initRamDiskButton, SIGNAL(clicked()), this, SLOT(setInitRamdiskPage()));
    connect(ui.bootloaderSettingsButton, SIGNAL(clicked()), this, SLOT(setBootloaderPage()));
    // pkg installer
    connect(ui.pkgList, SIGNAL(currentRowChanged(int)), this, SLOT(currentPkgItemChanged(int)));
    connect(ui.pkgInstallButton, SIGNAL(clicked()), this, SLOT(pkgInstallButtonClicked()));
    ui.pkgScreenLabel->installEventFilter(this);
    ui.screenshotLabel->installEventFilter(this);
    // bundle download
    connect(ui.bundlesDownloadButton, SIGNAL(clicked()), this, SLOT(bundlesDownloadButtonClicked()));
    // generate initrd
    connect(ui.generateInitRamDiskButton, SIGNAL(clicked()), this, SLOT(generateInitRamDisk()));

    // page icons
    ui.installPkgzButton->setIcon(KIcon("repository"));
    ui.downloadBundlesButton->setIcon(KIcon("x-cb-bundle"));
    ui.changeAppearanceButton->setIcon(KIcon("preferences-desktop-color"));
    ui.initRamDiskButton->setIcon(KIcon("cpu"));
    ui.bootloaderSettingsButton->setIcon(KIcon("go-first"));
    // pkg installer
    ui.pkgInstallButton->setIcon(KIcon("run-build-install"));
    // bundle download
    ui.bundlesDownloadButton->setIcon(KIcon("download"));
    // generate initrd
    ui.generateInitRamDiskButton->setIcon(KIcon("debug-run"));

    // enable usb & nfs, they are initrd built-in
    ui.usb->setChecked(true);
    ui.nfs->setChecked(true);

    // remove the initrd tmp files
    QProcess::execute("bash -c \"rm " + tmpInitRd.join(" ") + " > /dev/null 2&>1\"");

    populatePkgzList();
    populateBundlesList();
}

bool ConfigPage::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        if (obj == ui.pkgScreenLabel) {
            ui.stackedWidget->setCurrentIndex(5);
        } else if (obj == ui.screenshotLabel) {
            ui.stackedWidget->setCurrentIndex(2);
        }
    }
    
    return 0;
}

void ConfigPage::switchPkgScreenshot()
{
    if (ui.stackedWidget->currentIndex() == 5)
        ui.stackedWidget->setCurrentIndex(2);
    else if (ui.stackedWidget->currentIndex() == 2)
        ui.stackedWidget->setCurrentIndex(5);
}

void ConfigPage::populatePkgzList()
{
    ui.pkgList->clear();

    QFile pkglistFile(QString(CONFIG_INSTALL_PATH) + "/configPagePkgData");

    if (pkglistFile.open(QIODevice::ReadOnly)) {
        m_incomingList = QString(pkglistFile.readAll()).trimmed().split("\n");
    } else {
        qDebug() << pkglistFile.errorString();
    }

    if (m_incomingList.isEmpty())
        return;

    foreach (QString pkg, m_incomingList) {
        QListWidgetItem *item = new QListWidgetItem(ui.pkgList);
        item->setSizeHint(QSize(0, 22));
        item->setText(pkg.split("::").at(1));
        item->setData(60, pkg.split("::").at(0));
        item->setData(61, pkg.split("::").at(2));
        item->setIcon(QIcon(QString(CONFIG_INSTALL_PATH) + "/" + item->data(60).toString() + ".png"));
        ui.pkgList->addItem(item);
        m_incomingList.append(pkg.split("::").at(0) + "_thumb");
        m_incomingList.append(pkg.split("::").at(0));
    }

    m_incomingIncr = 0;
    m_incomingExtension = ".jpeg";

    KUrl r(QUrl("http://chakra-project.org/packages/screenshots/" + 
                m_incomingList.at(m_incomingIncr) + m_incomingExtension));
    m_job = KIO::get(r, KIO::Reload, KIO::Overwrite | KIO::HideProgressInfo);
    connect(m_job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(incomingData(KIO::Job*, QByteArray)));
}

void ConfigPage::incomingData(KIO::Job* job, QByteArray data)
{
    if (data.isNull()) {
        if (job->processedAmount(KJob::Bytes) == job->totalAmount(KJob::Bytes))
            downloadComplete();

        return;
    }

    if (m_incomingExtension == ".jpeg") {
        QFile x("/tmp/" + m_incomingList.at(m_incomingIncr) + m_incomingExtension);
        if (x.open(QIODevice::Append)) {
            x.write(data);
            x.flush();
        }
    } else {
        QFile x("/home/" + m_install->userLoginList().first() + "/Desktop/" + m_incomingList.at(m_incomingIncr));
        if (x.open(QIODevice::Append)) {
            x.write(data);
            x.flush();
        }
    }
}

void ConfigPage::downloadComplete()
{
    m_incomingIncr++;

    if (m_incomingIncr == m_incomingList.count()) {
        m_incomingIncr = 0;
        m_incomingList.clear();
        m_incomingExtension = "";
        return;
    }

    KUrl r(QUrl("http://chakra-project.org/packages/screenshots/" + 
                m_incomingList.at(m_incomingIncr) + m_incomingExtension));
    m_job = KIO::get(r, KIO::Reload, KIO::Overwrite | KIO::HideProgressInfo);
    connect(m_job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(incomingData(KIO::Job*, QByteArray)));
}

void ConfigPage::populateBundlesList()
{
    ui.bundlesList->clear();

    QStringList bundleDataList;
    QFile bundlelistFile(QString(CONFIG_INSTALL_PATH) + "/configPageBundleData");

    if (bundlelistFile.open(QIODevice::ReadOnly)) {
        bundleDataList = QString(bundlelistFile.readAll()).trimmed().split("\n");
    } else {
        qDebug() << bundlelistFile.errorString();
    }
    
    if (bundleDataList.isEmpty())
        return;

    foreach (QString pkg, bundleDataList) {
        QListWidgetItem *item = new QListWidgetItem(ui.bundlesList);
        item->setSizeHint(QSize(160, 36));
        item->setText(pkg.split("::").at(1));
        item->setCheckState(Qt::Unchecked);
        item->setData(60, pkg.split("::").at(0));
        item->setData(61, pkg.split("::").at(2));
        item->setIcon(QIcon(QString(CONFIG_INSTALL_PATH) + "/" + item->data(60).toString() + ".png"));
        ui.bundlesList->addItem(item);
    }
}

void ConfigPage::bundlesDownloadButtonClicked()
{
    m_incomingList.clear();
    m_incomingExtension = "";
    m_incomingIncr = 0;

    QStringList checkedList;
    
    for ( int i = 0; i < ui.bundlesList->count(); i++ ) {
        if (ui.bundlesList->item(i)->checkState() == Qt::Checked) {
            checkedList.append(ui.bundlesList->item(i)->data(60).toString());
        }
    }

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    // check stable/testing
    QFile pacmanConf("/etc/pacman.conf", this);
    pacmanConf.open(QIODevice::ReadOnly);
    QString sPacmanConf(pacmanConf.readAll());
    pacmanConf.close();

    foreach (QString line, sPacmanConf.split("\n")) {
        if (line.contains("-testing")) {
            if (line.simplified().trimmed().startsWith("#"))
                continue;

            m_currentBranch = "-testing";
        }
    }

    // check uname for arch
    m_process->start("uname -m");
    m_process->waitForFinished();
    if (m_process->readAll().contains("x86_64")) {
        m_currentArch = "x86_64";
    } else {
        m_currentArch = "i686";
    }

    // call rsync
    foreach (QString bundle, checkedList) {
        m_process->start("bash -c \"echo $(rsync -avh --list-only cinstall@chakra-project.org::cinstall/bundles" +
                        m_currentBranch + "/" + m_currentArch +
                        "/*  | cut -d \':\' -f 3 | cut -d \' \' -f 2 | grep " + 
                        bundle + ")\"");
        m_process->waitForFinished();
        QString result(m_process->readAll());
        if (result.simplified().trimmed().split(" ").count() > 1) {
            m_incomingList.append(result.simplified().trimmed().split(" ").last());
        } else {
            m_incomingList.append(result);
        }
    }

    KUrl r(QUrl("http://chakra-project.org/repo/bundles" + m_currentBranch + "/" +
                    m_currentArch + "/" + m_incomingList.at(m_incomingIncr)));
    m_job = KIO::get(r, KIO::Reload, KIO::Overwrite | KIO::HideProgressInfo);
    connect(m_job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(incomingData(KIO::Job*, QByteArray)));
}

void ConfigPage::currentPkgItemChanged(int i)
{
    // pkg name
    ui.pkgNameLabel->setText("<font size=5><b>" + ui.pkgList->item(i)->data(60).toString());

    // pkg version
    QProcess p;
    p.start("pacman -Sp --print-format %v " + ui.pkgList->item(i)->data(60).toString());
    p.waitForFinished();
    QString pkgVer = QString(p.readAll()).simplified().split(" ").last();
    p.start("chroot " + QString(INSTALLATION_TARGET) + " pacman -Qq");
    p.waitForFinished();
    QString installedPkgz(p.readAll());
    if (installedPkgz.contains(ui.pkgList->item(i)->data(60).toString()))
        pkgVer.append("  (" + i18n("installed") + ")");
    ui.pkgVerLabel->setText(pkgVer);

    // pkg description
    ui.pkgDescLabel->setText(ui.pkgList->item(i)->data(61).toString());

    // pkg screenshot
    ui.pkgScreenLabel->setPixmap(QPixmap("/tmp/" + ui.pkgList->item(i)->data(60).toString() + "_thumb.jpeg"));
    ui.screenshotLabel->setPixmap(QPixmap("/tmp/" + ui.pkgList->item(i)->data(60).toString() + ".jpeg"));
}

void ConfigPage::setInstallPkgzPage()
{
    if (ui.stackedWidget->currentIndex() != 2) {
        ui.stackedWidget->setCurrentIndex(2);
        ui.currentPageLabel->setText(i18n("Install Packaged Software"));
    } else {
        ui.stackedWidget->setCurrentIndex(0);
        ui.currentPageLabel->setText("");
    }
}

void ConfigPage::pkgInstallButtonClicked()
{
    // disable buttons
    ui.pkgInstallButton->setEnabled(false);
    enableNextButton(false);
    // mount special folders
    QProcess::execute("bash -c \"mount -v -t proc none " + QString(INSTALLATION_TARGET) + "/proc > /dev/null 2&>1\"");
    QProcess::execute("bash -c \"mount -v -t sysfs none " + QString(INSTALLATION_TARGET) + "/sys > /dev/null 2&>1\"");
    QProcess::execute("bash -c \"mount -v -o bind /dev " + QString(INSTALLATION_TARGET) + "/dev > /dev/null 2&>1\"");
    QProcess::execute("bash -c \"mount -v -t devpts devpts " + QString(INSTALLATION_TARGET) + "/dev/pts > /dev/null 2&>1\"");
    QProcess::execute("bash -c \"xhost +\" > /dev/null 2&>1");
    // cinstall pkg
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processComplete()));
    m_process->start("chroot " + QString(INSTALLATION_TARGET) + " su " + m_install->userLoginList().first() + " -c \"cinstall -i " + ui.pkgList->currentItem()->data(60).toString() + "\"");
}

void ConfigPage::processComplete()
{
    // clean-up
    QProcess::execute("umount -v " + QString(INSTALLATION_TARGET) + "/proc " + 
                                     QString(INSTALLATION_TARGET) + "/sys "  + 
                                     QString(INSTALLATION_TARGET) + "/dev/pts " + 
                                     QString(INSTALLATION_TARGET) + "/dev");
    // re-enable buttons
    ui.pkgInstallButton->setEnabled(true);
    enableNextButton(true);
}

void ConfigPage::setDownloadBundlesPage()
{
    if (ui.stackedWidget->currentIndex() != 3) {
        ui.stackedWidget->setCurrentIndex(3);
        ui.currentPageLabel->setText(i18n("Download Popular Bundles"));
    } else {
        ui.stackedWidget->setCurrentIndex(0);
        ui.currentPageLabel->setText("");
    }
}

void ConfigPage::setChangeAppearancePage()
{

}

void ConfigPage::setBootloaderPage()
{
    if (ui.stackedWidget->currentIndex() != 6) {
        ui.stackedWidget->setCurrentIndex(6);
        ui.currentPageLabel->setText(i18n("Bootloader Settings"));
    } else {
        ui.stackedWidget->setCurrentIndex(0);
        ui.currentPageLabel->setText("");
    }
}

void ConfigPage::setInitRamdiskPage()
{
    if (ui.stackedWidget->currentIndex() != 1) {
        ui.stackedWidget->setCurrentIndex(1);
        ui.currentPageLabel->setText(i18n("Customize Initial Ramdisk"));
    } else {
        ui.stackedWidget->setCurrentIndex(0);
        ui.currentPageLabel->setText("");
    }
}

void ConfigPage::generateInitRamDisk()
{
    ui.generateInitRamDiskButton->setEnabled(false);
    enableNextButton(false);
    m_busyAnim = new QMovie(":Images/images/busywidget.gif");
    m_busyAnim->start();
    ui.initRdBusyLabel->setMovie(m_busyAnim);
    ui.initRdBusyLabel->setVisible(true);

    if (ui.usb->isChecked())
        QProcess::execute("touch " + tmpInitRd.at(0));
    if (ui.firewire->isChecked())
        QProcess::execute("touch " + tmpInitRd.at(1));
    if (ui.pcmcia->isChecked())
        QProcess::execute("touch " + tmpInitRd.at(2));
    if (ui.nfs->isChecked())
        QProcess::execute("touch " + tmpInitRd.at(3));
    if (ui.raid->isChecked())
        QProcess::execute("touch " + tmpInitRd.at(4));
    if (ui.mdp->isChecked())
        QProcess::execute("touch " + tmpInitRd.at(5));
    if (ui.lvm2->isChecked())
        QProcess::execute("touch " + tmpInitRd.at(6));
    if (ui.encrypted->isChecked())
        QProcess::execute("touch " + tmpInitRd.at(7));
    
    QString command  = QString("sh " + QString(SCRIPTS_INSTALL_PATH) +
                               "/postinstall.sh --job create-initrd %1")
                               .arg(m_install->m_postcommand);
    connect(m_process, SIGNAL(finished(int)), this, SLOT(initRdGenerationComplete()));
    m_process->start(command);
}

void ConfigPage::initRdGenerationComplete()
{
    ui.generateInitRamDiskButton->setEnabled(true);
    enableNextButton(true);
    ui.initRdBusyLabel->setVisible(false);

    // remove tmp files
    QProcess::execute("bash -c \"rm " + tmpInitRd.join(" ") + " > /dev/null 2&>1\"");
}

void ConfigPage::bootloaderInstalled(int exitCode, QProcess::ExitStatus exitStatus)
{
    disconnect(m_install, SIGNAL(bootloaderInstalled(int, QProcess::ExitStatus)), 0, 0);

    Q_UNUSED(exitStatus)

    if (exitCode != 0) {

    } else {
        connect(m_install, SIGNAL(bootloaderInstalled(int, QProcess::ExitStatus)), SLOT(menulstInstalled(int, QProcess::ExitStatus)));

        emit setProgressWidgetText(i18n("Creating OS list..."));
        emit updateProgressWidget(50);

        m_install->installBootloader(1, "0");  /// hardcoding fail
    }
}

void ConfigPage::menulstInstalled(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)

    if (exitCode != 0) {

    } else {
        emit deleteProgressWidget();
        emit goToNextStep();
    }
}

void ConfigPage::aboutToGoToNext()
{
    ui.stackedWidget->setCurrentIndex(0);

    if (!ui.bootloaderCheckBox->isChecked()) {
        emit goToNextStep();
        return;
    }

    emit showProgressWidget();
    emit setProgressWidgetText(i18n("Installing bootloader..."));
    emit updateProgressWidget(0);

    connect(m_install, SIGNAL(bootloaderInstalled(int, QProcess::ExitStatus)), SLOT(bootloaderInstalled(int, QProcess::ExitStatus)));

    m_install->installBootloader(0, "0"); /// NOTE!! this is the hardcoded 'first disk only' bug
                                                 /// or where it starts rather.
}

void ConfigPage::aboutToGoToPrevious()
{
    emit goToPreviousStep();
}

#include "configpage.moc"
