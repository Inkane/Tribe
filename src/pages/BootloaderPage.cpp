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

#include "BootloaderPage.h"

#include "../InstallationHandler.h"

#include <KDebug>

BootloaderPage::BootloaderPage(QWidget *parent)
        : AbstractPage(parent),
        m_handler(InstallationHandler::instance())
{
    qDebug() << "BootloaderPage::BootloaderPage()";
}

BootloaderPage::~BootloaderPage()
{
}

void BootloaderPage::createWidget()
{
    ui.setupUi(this);
    ui.label_3->hide();
    ui.lineEdit->hide();
}

void BootloaderPage::aboutToGoToNext()
{
    if (!ui.checkBox->isChecked()) {
        emit goToNextStep();
        return;
    }

    emit showProgressWidget();
    emit setProgressWidgetText(i18n("Installing bootloader..."));
    emit updateProgressWidget(0);

    connect(m_handler, SIGNAL(bootloaderInstalled(int, QProcess::ExitStatus)), SLOT(bootloaderInstalled(int, QProcess::ExitStatus)));

    QString device = ui.lineEdit->text();

    device.remove(0, 2);

    m_handler->installBootloader(0, device);
}

void BootloaderPage::aboutToGoToPrevious()
{
    emit goToPreviousStep();
}

void BootloaderPage::bootloaderInstalled(int exitCode, QProcess::ExitStatus exitStatus)
{
    disconnect(m_handler, SIGNAL(bootloaderInstalled(int, QProcess::ExitStatus)), 0, 0);

    kDebug() << "Bootloader installed";

    Q_UNUSED(exitStatus)

    if (exitCode != 0) {
        // Ouch!! Errors!!
        kDebug() << "Error installing bootloader";
    } else {
        // All went well, so let's move on!

        connect(m_handler, SIGNAL(bootloaderInstalled(int, QProcess::ExitStatus)), SLOT(menulstInstalled(int, QProcess::ExitStatus)));

        emit setProgressWidgetText(i18n("Creating menu.lst..."));
        emit updateProgressWidget(50);

        QString device = ui.lineEdit->text();

        device.remove(0, 2);

        m_handler->installBootloader(1, device);
    }
}

void BootloaderPage::menulstInstalled(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)

    if (exitCode != 0) {
        // Ouch!! Errors!!
    } else {
        // All went well, so let's move on!

        emit deleteProgressWidget();

        emit goToNextStep();
    }
}

#include "BootloaderPage.moc"
