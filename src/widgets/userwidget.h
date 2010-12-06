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

#ifndef USERWIDGET_H
#define USERWIDGET_H

#include "avatardialog.h"
#include "ui_userwidget.h"


class UserWidget : public QWidget
{
    Q_OBJECT

public:
    UserWidget(int, QWidget *parent = 0);
    virtual ~UserWidget();

    QString login;
    QString password;
    QString rootPassword;
    QString avatar;
    QString name;
    bool autoLogin;
    bool admin;
    bool passwordsMatch;
    bool rootPasswordsMatch;

    int number;
    void setNumber(int i) { number = i; }
    
    void setAutoLogin(bool);

signals:
    void addUserClicked();
    void removeUserClicked(int);
    void autoLoginToggled(int);

private slots:
    void showDetails();
    void showRootPw();

    void emitRemove();

    void avatarClicked();
    void setAvatar(QString);
    void autoLoginToggled();
    void adminToggled();

    void testFields();

private:
    Ui::UserWidget ui;
    AvatarDialog *m_avatarDialog;
};

#endif /* USERWIDGET_H */
