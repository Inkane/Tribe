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

#include <QList>
#include <QPixmap>
#include "installationpage.h"

namespace {

QList<QPixmap> g_screenshots;

} // namespace

InstallationPage::InstallationPage(QWidget *parent)
        : AbstractPage(parent)
        , m_install(InstallationHandler::instance())
        , m_index( 0 )
        , m_timer( 0 )
{
    connect(m_install, SIGNAL(streamProgress(int)), SLOT(setPercentage(int)));
    connect(m_install, SIGNAL(streamLabel(const QString&)), SLOT(setLabel(const QString&)));
    connect(m_install, SIGNAL(installationDone()), SLOT(aboutToGoToNext()));
    connect(m_install, SIGNAL(errorInstalling()), SLOT(error()));

    g_screenshots = QList<QPixmap>()
        << QPixmap(":/Images/images/screenshot01.png")
        << QPixmap(":/Images/images/screenshot02.png")
        << QPixmap(":/Images/images/screenshot03.png")
        << QPixmap(":/Images/images/screenshot04.png")
        << QPixmap(":/Images/images/screenshot05.png")
        << QPixmap(":/Images/images/screenshot06.png")
        << QPixmap(":/Images/images/screenshot07.png")
        << QPixmap(":/Images/images/screenshot08.png")
        << QPixmap(":/Images/images/screenshot09.png")
        << QPixmap(":/Images/images/screenshot10.png");

    m_timer = new QTimer();
    m_timer->setInterval(5000);
}

InstallationPage::~InstallationPage()
{
}

void InstallationPage::createWidget()
{
    ui.setupUi(this);

    ui.comboBox->hide();

    connect(m_timer, SIGNAL(timeout()), SLOT(changeScreenShot()));

    changeScreenShot();

    m_timer->start();

    // Start the madness!!

    m_install->installSystem();
}

void InstallationPage::setPercentage(int percentage)
{
    ui.progressBar->setValue(percentage);
}

void InstallationPage::setLabel(const QString &label)
{
    ui.statusLabel->setText(label);
}

void InstallationPage::error()
{

}

void InstallationPage::aboutToGoToNext()
{
    emit goToNextStep();
}

void InstallationPage::aboutToGoToPrevious()
{
    // Don't emit anything here!! We can't go previous in this page.
}

void InstallationPage::changeScreenShot()
{
    /* Screenshot rotation!! Let's define how it will happen
     */

    if (g_screenshots.isEmpty()) {
        return;
    }

    ui.imageHolder->setPixmap(g_screenshots.at(m_index));
    m_index = (m_index + 1) % g_screenshots.count();
}

#include "installationpage.moc"
