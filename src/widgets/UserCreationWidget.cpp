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

#include "UserCreationWidget.h"

#include <QRegExpValidator>

static const char* USER_NAME_REGEXP = "[a-z0-9-]*";

UserCreationWidget::UserCreationWidget(QWidget *parent)
        : QWidget(parent)
{
    setupUi(this);

    deleteButton->setIcon(KIcon("list-remove"));
    sudoBox->setVisible(false);

    QRegExpValidator *v = new QRegExpValidator(QRegExp(USER_NAME_REGEXP), this);
    usernameEdit->setValidator(v);

    connect(deleteButton, SIGNAL(clicked()), SLOT(emitDestroy()));
    connect(passwordEdit, SIGNAL(textEdited(const QString&)), SLOT(passwordChanged()));
    connect(confirmEdit, SIGNAL(textEdited(const QString&)), SLOT(passwordChanged()));
    nameEdit->installEventFilter(this);
    passwordChanged();
}

UserCreationWidget::~UserCreationWidget()
{
}

void UserCreationWidget::passwordChanged()
{
    if (passwordEdit->text().isEmpty()) {
        statusLabel->setText(i18n("Password is empty"));
        iconLabel->setPixmap(KIcon("dialog-warning").pixmap(18, 18));
        emit widgetValidation();
        return;
    }

    if (passwordEdit->text() != confirmEdit->text()) {
        statusLabel->setText(i18n("Passwords do not match"));
        iconLabel->setPixmap(KIcon("dialog-warning").pixmap(18, 18));
        emit widgetValidation();
        return;
    }

    if (passwordEdit->text() == confirmEdit->text()) {
        statusLabel->setText(i18n("Passwords match"));
        iconLabel->setPixmap(KIcon("dialog-ok-apply").pixmap(18, 18));
        emit widgetValidation();
        return;
    }
}

bool UserCreationWidget::isPasswordValid()
{
    if (passwordEdit->text() == confirmEdit->text())
        return true;
    else
        return false;
}

bool UserCreationWidget::eventFilter(QObject * watched, QEvent * event)
{
    if (watched == nameEdit && event->type() == QEvent::FocusOut && usernameEdit->text().isEmpty()) {
        usernameEdit->setText(nameEdit->text().toLower());
    }
    return QObject::eventFilter(watched, event);
}

bool UserCreationWidget::isRemovable() const
{
    return deleteButton->isEnabled();
}

void UserCreationWidget::setRemovable(bool removeable)
{
    deleteButton->setEnabled(removeable);
}

#include "UserCreationWidget.moc"
