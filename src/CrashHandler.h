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

#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <QObject>
#include <QLatin1String>

class CrashHandler
{
    public:
        // Activate the handler
        static void activate();
        // The crash handler C function
        static void tribeCrashed(int signal);
        // Assign the app name
        static void setAppName(const QLatin1String &appName);

    private:
        static QLatin1String m_appName;
};

class HandlerActions : public QObject
{
    Q_OBJECT

    public:
        void setBacktrace(const QString &b);
    private Q_SLOTS:
        void copyToClipboard();
        void sendBacktrace();

    private:
        QString m_backtrace;
        bool m_sent;
};

#endif /* CRASHHANDLER_H */
