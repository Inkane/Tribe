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

#ifndef INSTALLATIONHANDLER_H
#define INSTALLATIONHANDLER_H

#include <QObject>
#include <QTime>
#include <QProcess>
#include <QMap>
#include <QPointer>
#include <QThread>
#include <klocalizedstring.h>
#include <kio/copyjob.h>

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

public:
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

    void setUpUser(const QString &user);

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

    void setUserName(const QString& s) { m_userName = s; }
    QString userName() { return m_userName; }

    void setUserLogin(const QString& s) { m_userLogin = s; }
    QString userLogin() { return m_userLogin; }

    void setUserPassword(const QString& s) { m_userPassword = s; }
    QString userPassword() { return m_userPassword; }

    void setRootPassword(const QString& s) { m_rootPassword = s; }
    QString rootPassword() { return m_rootPassword; }

    void setConfigurePacman(bool pac) { m_configurePacman = pac; }
    bool configurePacman() { return m_configurePacman; }

    void setRootDevice(const QString& s) { m_rootDevice = s; }
    QString rootDevice() { return m_rootDevice; }

public slots:
    void cleanup();

private:
    InstallationHandler(QObject *parent = 0);
    void installHome();
    void installKDEConfiguration();

    void handleProgress(CurrentAction act, int percentage);

    int antiFlicker();

    QString trimDevice(const Partition *device);

    bool isMounted(const QString &partition);

private Q_SLOTS:
    void parseUnsquashfsOutput();

    void jobDone(int result);
    void postInstall();
    void postInstallDone(int eC, QProcess::ExitStatus eS);

    void reconnectJobSlot();

    void postRemove();

    void readyPost();

    void partitionMounted(KJob *job);

    void unmountAll();
    void killProcesses();

    void streamPassword();
    void streamRootPassword();

    void partitionsFormatted();

signals:
    void streamProgress(int percentage);
    void streamLabel(const QString &label);
    void installationDone();
    void errorInstalling(const QString &message = i18n("No Further details given"));

    void mounting(const QString &ent, InstallationHandler::Status status);

    void mountingDone();

    void bootloaderInstalled(int, QProcess::ExitStatus);

private:
    HomeAction hAction;
    QTime eTime;
    QMap<QString, const Partition*> m_mount;
    QMap<QString, const Partition*>::const_iterator m_mapIterator;
    QStringList::const_iterator m_stringlistIterator;
    QString m_postjob;
    QString m_postlabel;
    QString m_postcommand;
    QPointer<QProcess> m_process;
    QStringList m_mtab;
    QProcess *m_userProcess;
    QProcess *m_rootProcess;

    CurrentAction currAct;
    FileHandling fileAct;
    QStringList m_removeLicenses;
    QString m_hostname;
    QString m_timezone;
    QString m_userName;
    QString m_userLogin;
    QString m_userPassword;
    QString m_rootPassword;
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
