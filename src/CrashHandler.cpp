/***************************************************************************
 *   Copyright (C) 2008 - 2009 by Dario Freddi                             *
 *   drf54321@gmail.com                                                    *
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

#include "CrashHandler.h"

#include <config-tribe.h>

#include "ui_crashWindow.h"

#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid()
#include <unistd.h>     // getpid(), alarm()

#include <QClipboard>
#include <QFile>
#include <QDateTime>

#include <KMessageBox>
#include <KDebug>
#include <KCrash>
#include <KProcess>
#include <kdeversion.h>
#include <KIO/NetAccess>
#include <KIO/CopyJob>

QLatin1String CrashHandler::m_appName("tribe");

void CrashHandler::activate()
{
#ifdef Q_WS_X11
    KCrash::setCrashHandler(CrashHandler::tribeCrashed);
#endif
}

void CrashHandler::tribeCrashed( int signal )
{
    Q_UNUSED( signal );

#ifdef Q_OS_UNIX
    /*
     * The main guts of this function are based
     * on the Amarok crash handler by Max Howell.
     */

    // Console message first.
    kWarning() << "Tribe crashed! -- this should not happen.\n"
    "Please submit a report at http://www.chakra-project.org/.\n"
    "\n"
    "Application version: " TRIBE_VERSION "\n"
    "Compiled at: KDE " KDE_VERSION_STRING ", Qt " QT_VERSION_STR "\n"
    "Running at:  KDE " + QLatin1String( KDE::versionString() ) + ", Qt " + QLatin1String( qVersion() ) + "\n";

    // Avoid loops
    KCrash::setCrashHandler();

    // We need to fork to be able to get a decent backtrace.
    // No idea why, perhaps some kdeinit magic causing trouble?
    const pid_t pid = ::fork();
    if( pid < 0 )
    {
        // fork failed
    }
    else if( pid > 0 )
    {
        // the parent, waits for the child.
        ::alarm( 0 );
        ::waitpid( pid, NULL, 0 );
        ::_exit( 253 );
    }

    KProcess gdb;
    gdb.setOutputChannelMode( KProcess::SeparateChannels );

    gdb << "gdb" << "--quiet"                 // avoid banners at startup
    << "--batch"                 // exit after processing
    << "--nw"                    // no window interface
    << "--nx"                    // don't read the .gdbinit file
    << "--ex" << "set width 0"   // no terminal width
    << "--ex" << "set height 0"  // no terminal height
    << "--ex" << "echo \\n==== (gdb) bt ====\\n"
    << "--ex" << "bt"
    //<< "--ex" << "echo ==== (gdb) thread apply all bt ====\\n"
    //<< "--ex" << "thread apply all bt"
    << m_appName << QByteArray::number( ::getppid() );    // attach to the process


/*    QString commandLine;
    QStringList fullCommand = gdb.program();
    foreach( const QString &arg, fullCommand )
    {
        if( ! commandLine.isEmpty() )
        {
            commandLine += " ";
        }

        commandLine += ( arg.contains(" ") ? "'" + arg + "'" : arg );
    }
    kDebug() << "full gdb command:" << commandLine;
*/

    // Start gdb.
    gdb.start();

    QString backtrace;
    bool showKdeBacktrace = false;

    // Wait maximum 30 seconds for gdb to finish.
    if( ! gdb.waitForFinished( 30000 ) )
    {
        // Something went wrong
        int returnCode = gdb.exitCode();

        if( returnCode == -2 )
        {
            kWarning() << "No backtrace could be generated, gdb not found. Printing KDE backtrace.";
        }
        else if( returnCode == -1 )
        {
            kWarning() << "No backtrace could be generated, gdb crashed.";
        }
        else if( returnCode != 0 )
        {
            kDebug() << "No backtrace could be generatd, gdb returned: " << returnCode;
        }

        showKdeBacktrace = true;
    }
    else
    {
        // gdb finished, read and check its output.
        backtrace = QString::fromUtf8(gdb.readAll());

        if ( backtrace.contains( "No stack." )
                || backtrace.contains( "Backtrace stopped" ) )
        {
            showKdeBacktrace = true;
        }

        kError() << "Tribe backtrace:" << backtrace;
    }
    if( showKdeBacktrace )
    {
        backtrace = kBacktrace();
        kError() << "KDE backtrace:" << kBacktrace();
    }

    Ui::crashWindow ui;
    HandlerActions *actions = new HandlerActions;
    actions->setBacktrace(backtrace);
    QWidget *w = new QWidget();
    ui.setupUi(w);
    ui.textBrowser->setText(backtrace);
    ui.copyButton->setIcon(KIcon("edit-copy"));
    ui.sendButton->setIcon(KIcon("mail-send"));
    KDialog *dial = new KDialog();
    dial->setMainWidget(w);
    dial->setButtons(KDialog::Close);
    actions->connect(ui.copyButton, SIGNAL(clicked()), actions, SLOT(copyToClipboard()));
    actions->connect(ui.sendButton, SIGNAL(clicked()), actions, SLOT(sendBacktrace()));
    dial->exec();

#else
#warning Crash handler code is not implemented for this platform.
#endif

    //_exit() exits immediately, otherwise this
    //function is called repeatedly ad finitum
    ::_exit( 255 );
}



/**
 * @brief Assign the app binary name.
 *
 * This function is called from the main() method to make sure the exact binary
 * (as passed to <code>argv[0]</code>) is known by the crash handler. This makes it possible
 * to run <code>gdb</code> properly. By default the appname is set to "tribe".
 *
 * @param  appName  Filename of the application binary.
 */
void CrashHandler::setAppName(const QLatin1String &appName)
{
    m_appName = appName;
}

void HandlerActions::setBacktrace(const QString &b)
{
    m_sent = false;
    m_backtrace = b;
}

void HandlerActions::copyToClipboard()
{
    QApplication::clipboard()->setText(m_backtrace);
}

void HandlerActions::sendBacktrace()
{
    if (m_sent) {
        KMessageBox::sorry(0, i18n("We appreciate your dedication, but your backtrace has already been "
                                   "successfully sent"));
    }

    QString filename = QString("/tmp/%1.trace").arg(QDateTime::currentDateTime().toString("dd-mm-yyyy_hhmm"));
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << m_backtrace;
    file.flush();
    file.close();

    KIO::CopyJob *job = KIO::copy(KUrl::fromPath(filename), KUrl("ftp://chakra-project.org/backtraces"), KIO::HideProgressInfo);
    if (KIO::NetAccess::synchronousRun(job, 0)) {
        KMessageBox::information(0, i18n("Thanks! Your backtrace was successfully submitted. Now just wait "
                                         "for us to fix this!"));
        m_sent = true;
    } else {
        KMessageBox::error(0, i18n("There was an error while sending your backtrace!! Probably we could not "
                                   "connect to the server: you can retry, but the best thing to do would be "
                                   "saving the backtrace and attaching it to a bug report later."));
    }
}

#include "CrashHandler.moc"
