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

#include "TribeLauncher.h"

#include <config-tribe.h>

#include <QApplication>
#include <QDebug>
#include <QStringList>
#include <QLayout>
#include <QRect>
#include <QDesktopWidget>
#include <QTimer>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

#include <KMessageBox>
#include <KDebug>
#include <klocalizedstring.h>

#include <aqpm/Backend.h>

#include <PolicyKit/polkit-qt/Auth>
#include <QDBusConnectionInterface>

#include <solid/networking.h>

using namespace Aqpm;

TribeLauncher::TribeLauncher(QWidget *parent)
        : QWidget(parent)
{
    setupUi(this);

    yesButton->setIcon(KIcon("dialog-ok"));
    noButton->setIcon(KIcon("dialog-cancel"));
    detailLabel->setVisible(false);
    yesButton->setVisible(false);
    noButton->setVisible(false);

    adjustSize();

    // Let's center it

    QRect rect = QApplication::desktop()->geometry();
    move(rect.center() - this->rect().center());

    connect(Backend::instance(), SIGNAL(backendReady()), this, SLOT(startupLauncher()));
}

TribeLauncher::~TribeLauncher()
{
}

void TribeLauncher::startupLauncher()
{
    // Ok, let's configure our library
    Backend::instance()->safeInitialization();
    Backend::instance()->setShouldHandleAuthorization(true);

    // Do we have a career?
    if (Solid::Networking::status() == Solid::Networking::Connected ||
        Solid::Networking::status() == Solid::Networking::Unknown) {
        connect(Backend::instance(), SIGNAL(operationFinished(bool)), this, SLOT(checkTribe()));
        Backend::instance()->updateDatabase();
    } else {
        yesButton->setVisible(false);
        noButton->setVisible(false);
        detailLabel->setVisible(true);
        m_bar->setVisible(false);
        m_label->setVisible(false);

        detailLabel->setText(i18n("It looks like you do not have an active internet connection. "
                                  "Updates check will be skipped"));
        QTimer::singleShot(4000, this, SLOT(launchTribe()));
    }
}

void TribeLauncher::launchTribe()
{
    detailLabel->setText(i18n("Setting up Tribe..."));

    yesButton->setVisible(false);
    noButton->setVisible(false);
    detailLabel->setVisible(true);
    m_bar->setVisible(false);
    m_label->setVisible(false);

    QDBusMessage message;

    if (!PolkitQt::Auth::computeAndObtainAuth("org.chakraproject.tribe.launchtribe")) {
        qDebug() << "User unauthorized";
        detailLabel->setText(i18n("It looks like you were not authorized to launch Tribe. Please "
                                  "contact your system administrator for further details"));
        QTimer::singleShot(6000, QCoreApplication::instance(), SLOT(quit()));
        return;
    }

    if (QDBusConnection::systemBus().interface()->isServiceRegistered("org.chakraproject.tribelauncherworker")) {
        detailLabel->setText(i18n("It looks like an unclean start of Tribe has taken place. Please kill all instances of "
                                  "tribe, tribelauncher and tribelauncherworker and try again"));
        QTimer::singleShot(10000, QCoreApplication::instance(), SLOT(quit()));
        return;
    }

    // Unfortunately, we cannot rely on DBus to launch it since we need an X display
    connect(QDBusConnection::systemBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this, SLOT(serviceOwnerChanged(QString,QString,QString)));
    QProcess::startDetached(QString("sudo %1").arg(TRIBE_LAUNCHER_WORKER_EXECUTABLE));
}

void TribeLauncher::checkTribe()
{
    disconnect(Backend::instance(), SIGNAL(operationFinished(bool)), 0, 0);

    bool uTribe = false;
    bool uTribePM = false;
    bool uArxin = false;
    bool forceUpgrade = false;

    // First of all, are Tribe, Tribe partitionmanager and Arxin installed?

    if (!Backend::instance()->package(TRIBE_PACKAGE, QString())->isInstalled()) {
        uTribe = true;
        forceUpgrade = true;
    }

    if (!Backend::instance()->package(TRIBE_PARTITIONMANAGER_PACKAGE, QString())->isInstalled()) {
        uTribePM = true;
        forceUpgrade = true;
    }

    if (!Backend::instance()->package(ARXIN_PACKAGE, QString())->isInstalled()) {
        uArxin = true;
        forceUpgrade = true;
    }

    // Are they upgradeable, in case?

    if (Backend::instance()->upgradeablePackages().contains(Backend::instance()->package(TRIBE_PACKAGE, QString()))) {
        uTribe = true;
    }

    if (Backend::instance()->upgradeablePackages().contains(Backend::instance()->package(
                                                               TRIBE_PARTITIONMANAGER_PACKAGE, QString()))) {
        uTribePM = true;
    }

    if (Backend::instance()->upgradeablePackages().contains(Backend::instance()->package(ARXIN_PACKAGE, QString()))) {
        uArxin = true;
    }


     if (forceUpgrade) {
         // If one of the packages is missing, we simply upgrade without asking,
         // otherwise we'll just walk into an epic fail.
 
         upgradeTribe();
         return;
     }

    if (!uTribe && !uArxin && !uTribePM) {
        launchTribe();
        return;
    } else if (uTribe || uTribePM) {
        detailLabel->setText(i18n("A newer version of Tribe, the Chakra installer, is available."
                                  "\nUpgrading it before starting the installation is strongly advised. "
                                  "Do you want to perform the upgrade now?"));
    } else if (uArxin) {
        detailLabel->setText(i18n("A newer version of Arxin is available.\n"
                                  "Even though not fundamental, upgrading is recommended. "
                                  "Do you want to perform the upgrade now?"));
    }

    yesButton->setVisible(true);
    noButton->setVisible(true);
    detailLabel->setVisible(true);
    m_bar->setVisible(false);
    m_label->setVisible(false);

    connect (yesButton, SIGNAL(clicked(bool)), this, SLOT(upgradeTribe()));
    connect (noButton, SIGNAL(clicked(bool)), this, SLOT(launchTribe()));
}

void TribeLauncher::upgradeTribe()
{
    yesButton->setVisible(false);
    noButton->setVisible(false);
    detailLabel->setVisible(false);
    m_bar->setVisible(true);
    m_label->setVisible(true);

    m_label->setText(i18n("Please Wait, Upgrading Chakra Installer..."));

    Backend::instance()->clearQueue();
    Backend::instance()->addItemToQueue(TRIBE_PACKAGE, QueueItem::Sync);
    Backend::instance()->addItemToQueue(TRIBE_PARTITIONMANAGER_PACKAGE, QueueItem::Sync);
    Backend::instance()->addItemToQueue(ARXIN_PACKAGE, QueueItem::Sync);

    connect(Backend::instance(), SIGNAL(streamDlProg(QString,int,int,int,int,int)),
            SLOT(streamDlProg(QString,int,int,int,int,int)));
    connect(Backend::instance(), SIGNAL(streamTransProgress(Aqpm::Globals::TransactionProgress,QString,int,int,int)),
            SLOT(streamTransProgress(Aqpm::Globals::TransactionProgress,QString,int,int,int)));
    connect(Backend::instance(), SIGNAL(streamTransEvent(Aqpm::Globals::TransactionEvent,QVariantMap)),
            SLOT(changeStatus(Aqpm::Globals::TransactionEvent,QVariantMap)));
    connect(Backend::instance(), SIGNAL(operationFinished(bool)), this, SLOT(operationFinished(bool)));

    Backend::instance()->processQueue(Aqpm::Globals::AllDeps);
}

void TribeLauncher::reloadLauncher()
{
    QProcess::startDetached("tribelauncher");
    QCoreApplication::instance()->quit();
}

void TribeLauncher::streamDlProg(const QString &c, int bytedone, int bytetotal, int speed,
                                 int listdone, int listtotal)
{
    Q_UNUSED(c);
    Q_UNUSED(bytedone);
    Q_UNUSED(bytetotal);

    unsigned int eta_s = 0;

    if (speed != 0) {
        eta_s = (int)((listtotal - listdone) / (speed));
    }

    m_bar->setFormat(QString(i18nc("You just have to translate 'remaining' here. "
                                   "Leave everything else as it is.", "%p% (%1 KB/s, %2 remaining)",
                                   speed / 1024, KGlobal::locale()->formatDuration(eta_s * 1000))));
    m_bar->setMaximum(listtotal);
    m_bar->setRange(0, listtotal);
    m_bar->setValue(listdone);

}

void TribeLauncher::streamTransProgress(Aqpm::Globals::TransactionProgress event, const QString &pkgname, int percent,
                                        int howmany, int remain)
{
    Q_UNUSED(event);
    Q_UNUSED(pkgname);
    Q_UNUSED(howmany);
    Q_UNUSED(remain);

    m_bar->setFormat(i18n("Installing..."));
    m_bar->setRange(0, 100);
    m_bar->setValue(percent);
}

void TribeLauncher::changeStatus(Aqpm::Globals::TransactionEvent event, const QVariantMap &args)
{
    Q_UNUSED(args)

    switch (event) {
    case Aqpm::Globals::AddStart:
        m_label->setText(i18n("Please Wait, Installing Packages..."));
        break;
    case Aqpm::Globals::UpgradeStart:
        m_label->setText(i18n("Please Wait, Upgrading Packages..."));
        break;
    case Aqpm::Globals::RetrieveStart:
        m_label->setText(i18n("Please Wait, Downloading Packages..."));
        break;
        /* all the simple done events, with fallthrough for each */
    default:
        break;
    }
}

void TribeLauncher::operationFinished(bool result)
{
    if (!result) {
        detailLabel->setText(i18n("An error occurred while trying to upgrade.\nDo you want to run Tribe anyway?"));

        yesButton->setVisible(true);
        noButton->setVisible(true);
        detailLabel->setVisible(true);
        m_bar->setVisible(false);
        m_label->setVisible(false);

        connect (yesButton, SIGNAL(clicked(bool)), this, SLOT(launchTribe()));
        connect (noButton, SIGNAL(clicked(bool)), QCoreApplication::instance(), SLOT(quit()));
    } else {
        launchTribe();
    }
}

void TribeLauncher::serviceOwnerChanged(const QString& name, const QString& oldOwner, const QString& newOwner)
{
    Q_UNUSED(oldOwner)

    if (name == "org.chakraproject.tribelauncherworker" && !newOwner.isEmpty()) {
        detailLabel->setText(i18n("Please wait, Tribe is being started..."));

        disconnect(QDBusConnection::systemBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                   this, SLOT(serviceOwnerChanged(QString,QString,QString)));

        QDBusMessage message = QDBusMessage::createMethodCall("org.chakraproject.tribelauncherworker",
                "/Worker",
                "org.chakraproject.tribelauncherworker",
                QLatin1String("launchTribe"));
        QDBusMessage rpl = QDBusConnection::systemBus().call(message, QDBus::NoBlock);
        if (rpl.type() == QDBusMessage::ErrorMessage) {
            qDebug() << rpl.errorMessage();
        }

        QTimer::singleShot(20000, QCoreApplication::instance(), SLOT(quit()));
    }
}

#include "TribeLauncher.moc"
