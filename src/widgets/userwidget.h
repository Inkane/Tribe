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

#include "ui_userwidget.h"


class UserWidget : public QWidget
{
    Q_OBJECT

public:
    UserWidget(int, QWidget *parent = 0);
    virtual ~UserWidget();

    QString login;
    QString password;
    QString avatar;
    QString name;
    bool autoLogin;
    bool admin;

signals:
    void addUserClicked();
    void removeUserClicked(int);

private slots:
    void showDetails();
    void emitRemove();

    void avatarClicked();
    void autoLoginToggled();
    void adminToggled();

    void testFields();

private:
    Ui::UserWidget ui;
    int m_userNumber;
};

#endif /* USERWIDGET_H */
