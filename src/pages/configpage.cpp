/*
 * Copyright (c) 2008, 2009  Dario Freddi <drf@chakra-project.org>
 *               2009        Lukas Appelhans <l.appelhans@gmx.de>
 *               2010        Drake Justice <djustice.kde@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <QDebug>

#include <KIcon>

#include <QProcess>
#include <QRegExpValidator>
#include <QFile>

#include <config-tribe.h>

#include "../installationhandler.h"
#include "configpage.h"
#include <QMovie>


const QString USB = "/tmp/tribe_initcpio_enable_usb";
const QString FIREWIRE = "/tmp/tribe_initcpio_enable_firewire";
const QString PCMCIA = "/tmp/tribe_initcpio_enable_pcmcia";
const QString NFS = "/tmp/tribe_initcpio_enable_nfs";
const QString SOFTWARE_RAID = "/tmp/tribe_initcpio_enable_softwareraid";
const QString SOFTWARE_RAID_MDP = "/tmp/tribe_initcpio_enable_softwareraidmdp";
const QString LVM2 = "/tmp/tribe_initcpio_enable_lvm2";
const QString ENCRYPTED = "/tmp/tribe_initcpio_enable_encrypted";


ConfigPage::ConfigPage(QWidget *parent)
        : AbstractPage(parent),
        m_install(InstallationHandler::instance())
{
}

ConfigPage::~ConfigPage()
{
}

void ConfigPage::createWidget()
{
    ui.setupUi(this);

    connect(ui.installPkgzButton, SIGNAL(clicked()), this, SLOT(setInstallPkgzPage()));
    connect(ui.downloadBundlesButton, SIGNAL(clicked()), this, SLOT(setDownloadBundlesPage()));
    connect(ui.changeAppearanceButton, SIGNAL(clicked()), this, SLOT(setChangeAppearancePage()));
    connect(ui.initRamDiskButton, SIGNAL(clicked()), this, SLOT(setInitRamDiskPage()));

    connect(ui.pkgList, SIGNAL(currentRowChanged(int)), this, SLOT(currentPkgItemChanged(int)));
    connect(ui.pkgInstallButton, SIGNAL(clicked()), this, SLOT(pkgInstallButtonClicked()));

    connect(ui.bundlesDownloadButton, SIGNAL(clicked()), this, SLOT(bundlesDownloadButtonClicked()));

    connect(ui.generateInitRamDiskButton, SIGNAL(clicked()), this, SLOT(generateInitRamDisk()));

    ui.installPkgzButton->setIcon(KIcon("repository"));
    ui.downloadBundlesButton->setIcon(KIcon("x-cb-bundle"));
    ui.changeAppearanceButton->setIcon(KIcon("preferences-desktop-color"));
    ui.initRamDiskButton->setIcon(KIcon("cpu"));
    
    ui.pkgInstallButton->setIcon(KIcon("run-build-install"));

    ui.bundlesList->setSelectionMode(QAbstractItemView::NoSelection);
    ui.bundlesDownloadButton->setIcon(KIcon("download"));
    
    ui.generateInitRamDiskButton->setIcon(KIcon("debug-run"));
    
    ui.usb->setChecked(true);
    ui.nfs->setChecked(true);

    QProcess::execute("rm " + USB);
    QProcess::execute("rm " + FIREWIRE);
    QProcess::execute("rm " + PCMCIA);
    QProcess::execute("rm " + NFS);
    QProcess::execute("rm " + SOFTWARE_RAID);
    QProcess::execute("rm " + SOFTWARE_RAID_MDP);
    QProcess::execute("rm " + LVM2);
    QProcess::execute("rm " + ENCRYPTED);
    
    populatePkgzList();
    populateBundlesList();
}

void ConfigPage::populatePkgzList()
{
    ui.pkgList->clear();

    QStringList pkgDataList;
    QFile pkglistFile(QString(CONFIG_INSTALL_PATH) + "/configPagePkgData");

    if (pkglistFile.open(QIODevice::ReadOnly)) {
        pkgDataList = QString(pkglistFile.readAll()).trimmed().split("\n");
    } else {
        qDebug() << pkglistFile.errorString();
    }
    
    if (pkgDataList.isEmpty())
        return;
    
    foreach (QString pkg, pkgDataList) {
        QListWidgetItem *item = new QListWidgetItem(ui.pkgList);
        item->setSizeHint(QSize(0, 22));
        item->setText(pkg.split("::").at(1));
        item->setData(60, pkg.split("::").at(0));
        item->setData(61, pkg.split("::").at(2));
        item->setIcon(KIcon(QString(CONFIG_INSTALL_PATH) + "/" + item->data(60).toString() + ".png"));
        ui.pkgList->addItem(item);
    }
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
        item->setSizeHint(QSize(100, 60));
        item->setText(pkg.split("::").at(1));
        item->setCheckState(Qt::Unchecked);
        item->setData(60, pkg.split("::").at(0));
        item->setData(61, pkg.split("::").at(2));
        item->setIcon(KIcon(QString(CONFIG_INSTALL_PATH) + "/" + item->data(60).toString() + ".png"));
        ui.bundlesList->addItem(item);
    }
}

void ConfigPage::bundlesDownloadButtonClicked()
{
    QStringList selectedBundles;
    
    for ( int i = 0; i < ui.bundlesList->count(); i++ ) {
        if (ui.bundlesList->item(i)->checkState() == Qt::Checked) {
            selectedBundles.append(ui.bundlesList->item(i)->data(60).toString());
        }
    }
}

void ConfigPage::currentPkgItemChanged(int i)
{
    ui.pkgNameLabel->setText("<font size=5><b>" + ui.pkgList->item(i)->data(60).toString());

    QProcess p;
    p.start("pacman -Sp --print-format %v " + ui.pkgList->item(i)->data(60).toString());
    p.waitForFinished();
    QString pkgVer = QString(p.readAll()).simplified().split(" ").last();
    ui.pkgVerLabel->setText(pkgVer);

    ui.pkgDescLabel->setText(ui.pkgList->item(i)->data(61).toString());
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
    QProcess::execute("xhost +");
    QProcess::execute("chroot " + QString(INSTALLATION_TARGET) + " su - " + m_install->userLoginList().first() + " -c \"cinstall -i " + ui.pkgList->currentItem()->data(60).toString() + "\"");
}

void ConfigPage::setDownloadBundlesPage()
{
    if (ui.stackedWidget->currentIndex() != 3) {
        ui.stackedWidget->setCurrentIndex(3);
        ui.currentPageLabel->setText(i18n("Download Bundled Software"));
    } else {
        ui.stackedWidget->setCurrentIndex(0);
        ui.currentPageLabel->setText("");
    }
}

void ConfigPage::setChangeAppearancePage()
{

}

void ConfigPage::setInitRamDiskPage()
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
    m_busyAnim = new QMovie(":Images/images/busywidget.gif");
    ui.initRdLabel->setMovie(m_busyAnim);

    if (ui.usb->isChecked())
        QProcess::execute("touch " + USB);
    if (ui.firewire->isChecked())
        QProcess::execute("touch " + FIREWIRE);
    if (ui.pcmcia->isChecked())
        QProcess::execute("touch " + PCMCIA);
    if (ui.nfs->isChecked())
        QProcess::execute("touch " + NFS);
    if (ui.raid->isChecked())
        QProcess::execute("touch " + SOFTWARE_RAID);
    if (ui.mdp->isChecked())
        QProcess::execute("touch " + SOFTWARE_RAID_MDP);
    if (ui.lvm2->isChecked())
        QProcess::execute("touch " + LVM2);
    if (ui.encrypted->isChecked())
        QProcess::execute("touch " + ENCRYPTED);
    
    QString command  = QString("sh " + QString(SCRIPTS_INSTALL_PATH) +
                               "/postinstall.sh --job create-initrd %1")
                               .arg(m_install->m_postcommand);
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int)), this, SLOT(initRdGenerationComplete()));
    m_process->start(command);
}

void ConfigPage::initRdGenerationComplete()
{
    ui.generateInitRamDiskButton->setEnabled(true);
    m_busyAnim = new QMovie(this);
    ui.initRdLabel->setMovie(m_busyAnim);

    QProcess::execute("rm " + USB);
    QProcess::execute("rm " + FIREWIRE);
    QProcess::execute("rm " + PCMCIA);
    QProcess::execute("rm " + NFS);
    QProcess::execute("rm " + SOFTWARE_RAID);
    QProcess::execute("rm " + SOFTWARE_RAID_MDP);
    QProcess::execute("rm " + LVM2);
    QProcess::execute("rm " + ENCRYPTED);
}

void ConfigPage::aboutToGoToNext()
{
    ui.stackedWidget->setCurrentIndex(2);

    emit goToNextStep();
}

void ConfigPage::aboutToGoToPrevious()
{
    emit goToPreviousStep();
}

#include "configpage.moc"
