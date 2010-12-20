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

#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include <QProcess>
#include <QMovie>

#include <KIO/NetAccess>
#include <KIO/TransferJob>
#include <KIO/Job>

#include "../abstractpage.h"
#include "ui_config.h"


class InstallationHandler;

class ConfigPage : public AbstractPage
{
    Q_OBJECT

public:
    ConfigPage(QWidget *parent = 0);
    virtual ~ConfigPage();

private slots:
    virtual void createWidget();
    virtual void aboutToGoToNext();
    virtual void aboutToGoToPrevious();
    
    bool eventFilter(QObject*, QEvent*);

    void setInstallPkgzPage();
    void setDownloadBundlesPage();
    void setChangeAppearancePage();
    void setInitRamdiskPage();
    void setBootloaderPage();

    void incomingData(KIO::Job*, QByteArray);
    void result(KJob*);
    void processComplete();

    // install pkg page
    void populatePkgzList();
    void currentPkgItemChanged(int);
    void pkgInstallButtonClicked();
    void switchPkgScreenshot();
    
    // download bundles page
    void populateBundlesList();
    void bundlesDownloadButtonClicked();

    // customize initrd page
    void initRdGenerationComplete();
    void generateInitRamDisk();
    
    // bootloader page
    void bootloaderInstalled(int, QProcess::ExitStatus);
    void menulstInstalled(int, QProcess::ExitStatus);

private:
    Ui::Config ui;

    InstallationHandler *m_install;

    QProcess *m_process;

    KIO::TransferJob *m_job;
    
    QMovie *m_busyAnim;

    QStringList m_incomingList;
    QString m_incomingExtension;
    int m_incomingIncr;

    QString m_currentBranch;
    QString m_currentArch;
};

#endif /* CONFIGPAGE_H */
