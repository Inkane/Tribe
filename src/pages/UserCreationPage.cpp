/***************************************************************************
 *   Copyright (C) 2008 - 2009 by Dario Freddi                             *
 *   drf@chakra-project.org                                                *
 *   Copyright (C) 2009 by Lukas Appelhans                                 *
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

#include <QDir>

#include <KIcon>
#include <KDebug>
#include <KMessageBox>
#include <KIO/Job>
#include <KIO/NetAccess>

#include <config-tribe.h>

#include "../InstallationHandler.h"
#include "UserCreationPage.h"

UserCreationPage::UserCreationPage(QWidget *parent)
        : AbstractPage(parent),
        m_handler(InstallationHandler::instance())
{
}

UserCreationPage::~UserCreationPage()
{
}

void UserCreationPage::createWidget()
{
    ui.setupUi(this);

    ui.nameLine->setFocus();

    ui.rootPw->setVisible(false);
    ui.rootPwLabel->setVisible(false);
    ui.rootConfirmPw->setVisible(false);
    ui.rootConfirmPwLabel->setVisible(false);

    connect(ui.usePwForRoot, SIGNAL(toggled(bool)), this, SLOT(toggleRootPw()));
}

void UserCreationPage::aboutToGoToNext()
{
    bool retbool;
    KDialog *dialog = new KDialog(this, Qt::FramelessWindowHint);
    dialog->setButtons(KDialog::Ok);
    dialog->setModal(true);

    if (ui.nameLine->text().isEmpty()) {
        KMessageBox::createKMessageBox(dialog, QMessageBox::Warning,
                                       i18n("You have to specify at least a username and password."),
                                       QStringList(), QString(), &retbool, KMessageBox::Notify);
        return;
    } else if (ui.userPw->text().isEmpty()) {
        KMessageBox::createKMessageBox(dialog, QMessageBox::Warning,
                                       i18n("You have to specify a password."),
                                       QStringList(), QString(), &retbool, KMessageBox::Notify);
        return;
    } else if (ui.userConfirmPw->text().isEmpty() || (ui.userPw->text() != ui.userConfirmPw->text())) {
        KMessageBox::createKMessageBox(dialog, QMessageBox::Warning,
                                       i18n("The passwords do not match."),
                                       QStringList(), QString(), &retbool, KMessageBox::Notify);
        return;
    } else if (!ui.usePwForRoot->isChecked()) {
        if (ui.rootPw->text().isEmpty()) {
            KMessageBox::createKMessageBox(dialog, QMessageBox::Warning,
                                           i18n("You have to specify a root password."),
                                           QStringList(), QString(), &retbool, KMessageBox::Notify);

            return;
        } else if (ui.rootPw->text() != ui.rootConfirmPw->text()) {
            KMessageBox::createKMessageBox(dialog, QMessageBox::Warning,
                                           i18n("The passwords do not match."),
                                           QStringList(), QString(), &retbool, KMessageBox::Notify);

            return;
        }
    }

    if (ui.userLine->text() != ui.userLine->text().toLower()) {
        KMessageBox::createKMessageBox(dialog, QMessageBox::Warning,
                                       i18n("Standard UNIX login names cannot contain capital letters. Using \"") + ui.userLine->text().toLower() + i18n("\" instead."),
                                       QStringList(), QString(), &retbool, KMessageBox::Notify);
    }

    m_handler->setUserLogin(ui.userLine->text().toLower());
    m_handler->setUserName(ui.nameLine->text());
    m_handler->setUserPassword(ui.userPw->text());
    m_handler->setRootPassword(ui.rootPw->text());

    emit goToNextStep();
}

void UserCreationPage::aboutToGoToPrevious()
{
    emit goToPreviousStep();
}


void UserCreationPage::toggleRootPw()
{
    ui.rootPw->setVisible(!ui.rootPw->isVisible());
    ui.rootPwLabel->setVisible(!ui.rootPwLabel->isVisible());
    ui.rootConfirmPw->setVisible(!ui.rootConfirmPw->isVisible());
    ui.rootConfirmPwLabel->setVisible(!ui.rootConfirmPwLabel->isVisible());
}

void UserCreationPage::validateNext()
{
    bool enable = true;

    if (ui.userLine->text().isEmpty()) { enable = false; }
    if (ui.userPw->text().isEmpty()) { enable = false; }
    if (ui.userPw->text() != ui.userConfirmPw->text()) { enable = false; }
    if (!ui.usePwForRoot->isChecked()) {
        if (ui.rootPw->text().isEmpty()) { enable = false; }
        if (ui.rootPw->text() != ui.rootConfirmPw->text()) { enable = false; }
    }

    emit enableNextButton(enable);
}

#include "UserCreationPage.moc"
