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

#ifndef TRIBELAUNCHER_H
#define TRIBELAUNCHER_H

#include <aqpm/Globals.h>

#include "ui_gui.h"

class TribeLauncher : public QWidget, private Ui::tribeLauncherUi
{
    Q_OBJECT

public:
    TribeLauncher(QWidget *parent = 0);
    virtual ~TribeLauncher();

private slots:
    void startupLauncher();

    void checkTribe();
    void upgradeTribe();
    void launchTribe();

    void streamDlProg(const QString &c, int bytedone, int bytetotal, int speed,
                      int listdone, int listtotal);

    void streamTransProgress(Aqpm::Globals::TransactionProgress event, const QString &pkgname,
                             int percent, int howmany, int remain);

    void changeStatus(Aqpm::Globals::TransactionEvent event, const QVariantMap &args);

    void operationFinished(bool result);

    void reloadLauncher();

    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
};

#endif /*TRIBELAUNCHER_H*/
