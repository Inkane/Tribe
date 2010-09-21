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

#ifndef USERCREATIONWIDGET_H
#define USERCREATIONWIDGET_H

#include "ui_createUserWidget.h"

class UserCreationWidget : public QWidget, private Ui::createUser
{
    Q_OBJECT

public:
    UserCreationWidget(QWidget *parent);
    virtual ~UserCreationWidget();

    QString getName() {
        return nameEdit->text() + ' ' + surnameEdit->text();
    }
    QString getUsername() {
        return usernameEdit->text();
    }
    QString getPassword() {
        return passwordEdit->text();
    }

    bool getSudo() {
        return sudoBox->isChecked();
    }

    bool isPasswordValid();

    bool isRemovable() const;
    void setRemovable(bool removeable);

protected:
    bool eventFilter(QObject * watched, QEvent * event);

private slots:
    void emitDestroy() {
        emit destroy(this);
    }
    void passwordChanged();

signals:
    void destroy(UserCreationWidget *widget);
    void widgetValidation();
};

#endif /*USERCREATIONWIDGET_H*/
