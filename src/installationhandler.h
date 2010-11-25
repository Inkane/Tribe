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

#ifndef INSTALLATIONHANDLER_H
#define INSTALLATIONHANDLER_H

#include <QObject>
#include <QTime>
#include <QProcess>
#include <QMap>
#include <QPointer>
#include <QThread>

#include <KLocalizedString>
#include <KIO/CopyJob>

class Partition;

class InstallationHandler : public QObject
{
    Q_OBJECT

public:

    enum HomeAction {
        KeepHome,
        OverwriteAll,
        OverwriteKDEConfigs
    };

    enum CurrentAction {
        DiskPreparation,
        SystemInstallation,
        PostInstall
    };

    enum FileHandling {
        OverwriteFile,
        KeepFile
    };

    enum Status {
        InProgress,
        Success,
        Error
    };

    virtual ~InstallationHandler();

    static InstallationHandler *instance();

    void init();
    void abortInstallation();

    qint64 minSizeForTarget() const;
    void addPartitionToMountList(const Partition *device, const QString &mountpoint);

    QString getMountPointFor(const QString &device);

    void clearMounts();

    void mountNextPartition();
    void unmountPartition(const QString &partition);
    QStringList getMountedPartitions();

    void setHomeBehaviour(HomeAction act);
    void setFileHandlingBehaviour(FileHandling fhnd);

    QStringList checkExistingHomeDirs();

    void installSystem();
    void copyFiles();

    void installBootloader(int action, const QString &device);

    void populateCommandParameters();

    QMap<QString, const Partition*> getMountPartitions() { return m_mount; }

    void resetIterators() {
        m_mapIterator = m_mount.constBegin();
        m_stringlistIterator = m_removeLicenses.constBegin();
    }

    void setUpUsers(QStringList);

    void setRemoveLicenses(const QStringList &packages) { m_removeLicenses = packages; }
    QStringList removeLicenses() { return m_removeLicenses; }

    void setHostname(const QString &name) { m_hostname = name; }
    QString hostname() { return m_hostname; }

    void setTimezone(const QString &time) { m_timezone = time; }
    QString timezone() { return m_timezone; }

    void setLocale(const QString &loc) { m_locale = loc; }
    QString locale() { return m_locale; }

    void setKDELangPack(const QString &pack) { m_KDELangPack = pack; }
    QString KDELangPack() { return m_KDELangPack; }

    void setInstallDocumentation(bool doc) { m_doc = doc; }
    bool installDocumentation() { return m_doc; }

    void setUserLoginList(QStringList s) { m_userLoginList = s; }
    QStringList userLoginList() { return m_userLoginList; }

    void setUserPasswordList(QStringList s) { m_userPasswordList = s; }
    QStringList userPasswordList() { return m_userPasswordList; }

    void setUserAvatarList(QStringList s) { m_userAvatarList = s; }
    QStringList userAvatarList() { return m_userAvatarList; }

    void setUserNameList(QStringList s) { m_userNameList = s; }
    QStringList userNameList() { return m_userNameList; }

    void setUserAutoLoginList(QStringList s) { m_userAutoLoginList = s; }
    QStringList userAutoLoginList() { return m_userAutoLoginList; }

    void setUserAdminList(QStringList s) { m_userAdminList = s; }
    QStringList userAdminList() { return m_userAdminList; }

    void setConfigurePacman(bool pac) { m_configurePacman = pac; }
    bool configurePacman() { return m_configurePacman; }

    void setRootDevice(const QString& s) { m_rootDevice = s; }
    QString rootDevice() { return m_rootDevice; }

public slots:
    void cleanup();

private Q_SLOTS:
    void postInstall();
    void postInstallDone(int eC, QProcess::ExitStatus eS);

    void parseUnsquashfsOutput();

    void jobDone(int result);
    void reconnectJobSlot();

    void postRemove();
    void readyPost();

    void partitionMounted(KJob *job);
    void unmountAll();
    void partitionsFormatted();

    void killProcesses();

    void streamPassword(int);

signals:
    void streamProgress(int percentage);
    void streamLabel(const QString &label);

    void installationDone();

    void errorInstalling(const QString &message = i18n("No Further details given"));

    void mounting(const QString &ent, InstallationHandler::Status status);
    void mountingDone();

    void bootloaderInstalled(int, QProcess::ExitStatus);

private:
    InstallationHandler(QObject *parent = 0);

    void installHome();
    void installKDEConfiguration();
    void handleProgress(CurrentAction act, int percentage);

    int antiFlicker();

    QString trimDevice(const Partition *device);

    bool isMounted(const QString &partition);

    QTime eTime;

    QMap<QString, const Partition*> m_mount;
    QMap<QString, const Partition*>::const_iterator m_mapIterator;

    QStringList::const_iterator m_stringlistIterator;

    QString m_postjob;
    QString m_postlabel;
    QString m_postcommand;

    QPointer<QProcess> m_process;
    QProcess *m_userProcess;

    HomeAction homeAction;
    CurrentAction currAction;
    FileHandling fileAction;

    QStringList m_mtab;

    QStringList m_removeLicenses;

    QStringList m_userLoginList;
    QStringList m_userPasswordList;
    QStringList m_userAvatarList;
    QStringList m_userNameList;
    QStringList m_userAdminList;
    QStringList m_userAutoLoginList;

    QString m_hostname;
    QString m_timezone;
    QString m_locale;
    QString m_KDELangPack;
    QString m_rootDevice;

    bool m_doc;
    bool m_configurePacman;

    int m_iterator;
    int m_unsquashFsCachedBottom;

    qint64 m_minSize;

    friend class PMHandler;
};

#endif /*INSTALLATIONHANDLER_H*/
