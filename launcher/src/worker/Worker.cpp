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

#include "Worker.h"

#include "tribelauncherworkeradaptor.h"

#include <QtDBus/QDBusConnection>
#include <QTimer>
#include <QDebug>

#include <PolicyKit/polkit-qt/Context>
#include <PolicyKit/polkit-qt/Auth>

Worker::Worker(QObject *parent)
        : QObject(parent)
{
    new TribelauncherworkerAdaptor(this);

    if (!QDBusConnection::systemBus().registerService("org.chakraproject.tribelauncherworker")) {
        qDebug() << "another helper is already running";
        QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        return;
    }

    if (!QDBusConnection::systemBus().registerObject("/Worker", this)) {
        qDebug() << "unable to register service interface to dbus";
        QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        return;
    }
}

Worker::~Worker()
{
}

void Worker::launchTribe()
{
    PolkitQt::Auth::Result result;
    result = PolkitQt::Auth::isCallerAuthorized("org.chakraproject.tribe.launchtribe",
             message().service(),
             true);
    if (result == PolkitQt::Auth::Yes) {
        qDebug() << message().service() << QString(" authorized");
    } else {
        qDebug() << QString("Not authorized");
        QCoreApplication::quit();
    }

    m_logfile = new QFile("/tmp/installation.log", this);

    if (!m_logfile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open log!!";
        QCoreApplication::quit();
    }

    m_stream.setDevice(m_logfile);

    m_process = new QProcess(this);

    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(logStderr()));
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(logStdout()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(cleanupAndClose()));

    m_process->start("bash");
    m_process->waitForStarted();
    m_process->write("kbuildsycoca4\n");
    m_process->write("tribe\n");
}

void Worker::logStderr()
{
    m_process->setReadChannel(QProcess::StandardError);

    m_stream << m_process->readLine();
}

void Worker::logStdout()
{
    m_process->setReadChannel(QProcess::StandardOutput);

    m_stream << m_process->readLine();
}

void Worker::cleanupAndClose()
{
    m_logfile->flush();
    m_logfile->close();
    m_logfile->deleteLater();

    m_process->deleteLater();

    QCoreApplication::instance()->quit();
}


