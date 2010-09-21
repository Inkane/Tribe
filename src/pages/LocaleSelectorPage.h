/***************************************************************************
 *   Copyright (C) 2008 by Lukas Appelhans                                 *
 *   l.appelhans@gmx.de                                                    *
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

#ifndef LOCALESELECTORPAGE_H
#define LOCALESELECTORPAGE_H

#include "../AbstractPage.h"

#include "ui_tribeLocale.h"

#include <QModelIndex>
#include <QHash>

class InstallationHandler;

class LocaleSelectorPage : public AbstractPage, public Ui::TribeLocale
{
    Q_OBJECT

public:
    LocaleSelectorPage(QWidget *parent = 0);
    virtual ~LocaleSelectorPage();

    bool eventFilter(QObject * object, QEvent * event);

private slots:
    void createWidget();
    void zoom(int value);
    void zoomChanged(int value);
    void timezoneChanged(int index);
    void updateLocales();
    void aboutToGoToPrevious();
    void aboutToGoToNext();

private:
    QList<QStringList> locales;
    InstallationHandler * m_install;
    QStringList m_allLocales;
    QHash<QString, QString> m_allKDELangs;
};

#endif /*LOCALESELECTORPAGE_H_*/

