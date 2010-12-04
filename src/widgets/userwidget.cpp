/*
 * Copyright (c) 2008, 2009  Dario Freddi <drf@chakra-project.org>
 *               2010        Drake Justice <djustice@chakra-project.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <QDebug>

#include <QDesktopWidget>

#include "avatardialog.h"
#include "userwidget.h"


UserWidget::UserWidget(int a_userNumber, QWidget* parent): QWidget(parent)
{
    number = a_userNumber;

    ui.setupUi(this);

    ui.extWidget->hide();

    ui.passLine->setEchoMode(QLineEdit::Password);
    ui.confirmPassLine->setEchoMode(QLineEdit::Password);

    ui.removeUser->setIcon(KIcon("list-remove"));
    ui.userDetails->setIcon(KIcon("view-list-details"));

    ui.avatar->setIconSize(QSize(48, 48));
    ui.avatar->setIcon(KIcon("view-user-offline-kopete"));

    m_avatarDialog = new AvatarDialog(0);

    if (number == 0) {
        autoLogin = true;
        admin = true;
        ui.autoLoginCheckBox->setChecked(true);
        ui.adminCheckBox->setChecked(true);
        ui.removeUser->setVisible(false);
    } else {
        autoLogin = false;
        admin = false;
    }

    passwordsMatch = true;

    connect(ui.loginLine, SIGNAL(textChanged(QString)), this, SLOT(testFields()));
    connect(ui.passLine, SIGNAL(textChanged(QString)), this, SLOT(testFields()));
    connect(ui.confirmPassLine, SIGNAL(textChanged(QString)), this, SLOT(testFields()));

    connect(ui.userDetails, SIGNAL(clicked(bool)), this, SLOT(showDetails()));
    connect(ui.removeUser, SIGNAL(clicked(bool)), this, SLOT(emitRemove()));

    connect(ui.avatar, SIGNAL(clicked(bool)), this, SLOT(avatarClicked()));
    connect(ui.autoLoginCheckBox, SIGNAL(toggled(bool)), this, SLOT(autoLoginToggled()));
    connect(ui.adminCheckBox, SIGNAL(toggled(bool)), this, SLOT(adminToggled()));
    
    connect(m_avatarDialog, SIGNAL(setAvatar(QString)), this, SLOT(setAvatar(QString)));
}

UserWidget::~UserWidget()
{

}

void UserWidget::setAvatar(QString a)
{
    if (a == "z") {
         
    } else {
        ui.avatar->setIcon(KIcon(a));
        avatar = a;
    }
}

void UserWidget::showDetails()
{
    ui.extWidget->setVisible(!ui.extWidget->isVisible());
}

void UserWidget::emitRemove()
{
    emit removeUserClicked(number);
}

void UserWidget::testFields()
{
    if ((ui.passLine->text() == ui.confirmPassLine->text()) &&
        (!ui.passLine->text().isEmpty())) {
        ui.confirmPwCheck->setPixmap(QPixmap(":Images/images/green-check.png"));
        password = ui.passLine->text();
        passwordsMatch = true;
    } else {
        ui.confirmPwCheck->setPixmap(QPixmap());
        passwordsMatch = false;
    }

    QRegExp r("\\D\\w{0,45}");
    QRegExp s("\\D\\w{0,45}[ ]");

    if (r.exactMatch(ui.loginLine->text())) {
        ui.loginLine->setText(ui.loginLine->text().toLower());
    } else if (s.exactMatch(ui.loginLine->text())) {
        ui.loginLine->setText(ui.loginLine->text().left(ui.loginLine->text().length() - 1));
    } else {
        ui.loginLine->setText("");
    }

    login = ui.loginLine->text();
    name = ui.nameLine->text();
    autoLogin = ui.autoLoginCheckBox->isChecked();
    admin = ui.adminCheckBox->isChecked();
}

void UserWidget::setAutoLogin(bool b)
{
    ui.autoLoginCheckBox->setChecked(b);
    autoLogin = b;
}

void UserWidget::avatarClicked()
{
    m_avatarDialog->show();
    m_avatarDialog->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, m_avatarDialog->size(), qApp->desktop()->availableGeometry()));
}

void UserWidget::autoLoginToggled()
{
    autoLogin = ui.autoLoginCheckBox->isChecked();
    if (autoLogin)
        emit autoLoginToggled(number);
}

void UserWidget::adminToggled()
{
    admin = ui.adminCheckBox->isChecked();
}


#include "userwidget.moc"
