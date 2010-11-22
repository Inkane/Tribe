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

#include <QProcess>
#include <QRegExpValidator>
#include <QFile>

#include "../installationhandler.h"
#include "settingspage.h"


static const char* HOST_NAME_REGEXP = "[a-z0-9-]*";

const QString USB = "/tmp/tribe_initcpio_enable_usb";
const QString FIREWIRE = "/tmp/tribe_initcpio_enable_firewire";
const QString PCMCIA = "/tmp/tribe_initcpio_enable_pcmcia";
const QString NFS = "/tmp/tribe_initcpio_enable_nfs";
const QString SOFTWARE_RAID = "/tmp/tribe_initcpio_enable_softwareraid";
const QString SOFTWARE_RAID_MDP = "/tmp/tribe_initcpio_enable_softwareraidmdp";
const QString LVM2 = "/tmp/tribe_initcpio_enable_lvm2";
const QString ENCRYPTED = "/tmp/tribe_initcpio_enable_encrypted";


SettingsPage::SettingsPage(QWidget *parent)
        : AbstractPage(parent),
        m_install(InstallationHandler::instance())
{
}

SettingsPage::~SettingsPage()
{
}

void SettingsPage::createWidget()
{
    ui.setupUi(this);

    ui.detect_mirror->setVisible(false);
    ui.download_doc->setVisible(false);

    connect(ui.pushButton, SIGNAL(toggled(bool)), ui.usb, SLOT(setVisible(bool)));
    connect(ui.pushButton, SIGNAL(toggled(bool)), ui.firewire, SLOT(setVisible(bool)));
    connect(ui.pushButton, SIGNAL(toggled(bool)), ui.pcmcia, SLOT(setVisible(bool)));
    connect(ui.pushButton, SIGNAL(toggled(bool)), ui.nfs, SLOT(setVisible(bool)));
    connect(ui.pushButton, SIGNAL(toggled(bool)), ui.raid, SLOT(setVisible(bool)));
    connect(ui.pushButton, SIGNAL(toggled(bool)), ui.mdp, SLOT(setVisible(bool)));
    connect(ui.pushButton, SIGNAL(toggled(bool)), ui.lvm2, SLOT(setVisible(bool)));
    connect(ui.pushButton, SIGNAL(toggled(bool)), ui.encrypted, SLOT(setVisible(bool)));

    ui.pushButton->setChecked(true);
    ui.pushButton->setChecked(false);
    ui.usb->setChecked(QFile::exists(USB));
    QFile::remove(USB);
    ui.firewire->setChecked(QFile::exists(FIREWIRE));
    QFile::remove(FIREWIRE);
    ui.pcmcia->setChecked(QFile::exists(PCMCIA));
    QFile::remove(PCMCIA);
    ui.nfs->setChecked(QFile::exists(NFS));
    QFile::remove(NFS);
    ui.raid->setChecked(QFile::exists(SOFTWARE_RAID));
    QFile::remove(SOFTWARE_RAID);
    ui.mdp->setChecked(QFile::exists(SOFTWARE_RAID_MDP));
    QFile::remove(SOFTWARE_RAID_MDP);
    ui.lvm2->setChecked(QFile::exists(LVM2));
    QFile::remove(LVM2);
    ui.encrypted->setChecked(QFile::exists(ENCRYPTED));
    QFile::remove(ENCRYPTED);

    QRegExpValidator *v = new QRegExpValidator(QRegExp(HOST_NAME_REGEXP), this);
    ui.hostnameEdit->setValidator(v);

    if (!m_install->hostname().isEmpty()) {
        ui.hostnameEdit->setText(m_install->hostname());
    } else if (QFile::exists("/tmp/platform-laptop")) {
        ui.hostnameEdit->setText(i18n("chakra-laptop"));
    } else if (QFile::exists("/tmp/platform-desktop")) {
        ui.hostnameEdit->setText(i18n("chakra-desktop"));
    }

    ui.hostnameEdit->setSelection(0, ui.hostnameEdit->text().indexOf("-"));
    ui.hostnameEdit->setFocus(Qt::TabFocusReason);

    ui.download_doc->setChecked(false);
    ui.detect_mirror->setChecked(false);

    connect(ui.hostnameEdit, SIGNAL(textChanged(QString)), SLOT(textChanged(QString)));
}

void SettingsPage::textChanged(const QString &string)
{
    if (string.isEmpty()) {
        emit enableNextButton(false);
    } else {
        emit enableNextButton(true);
    }
}

void SettingsPage::aboutToGoToNext()
{
    if (ui.usb->isChecked())
        QProcess::execute("touch " + USB);
    if (ui.firewire->isChecked())
        QProcess::execute("touch " + FIREWIRE);
    if (ui.pcmcia->isChecked())
        QProcess::execute("touch " + PCMCIA);
    if (ui.nfs->isChecked())
        QProcess::execute("touch " + NFS);
    if (ui.raid->isChecked())
        QProcess::execute("touch " + SOFTWARE_RAID);
    if (ui.mdp->isChecked())
        QProcess::execute("touch " + SOFTWARE_RAID_MDP);
    if (ui.lvm2->isChecked())
        QProcess::execute("touch " + LVM2);
    if (ui.encrypted->isChecked())
        QProcess::execute("touch " + ENCRYPTED);

    m_install->setConfigurePacman(ui.detect_mirror->isChecked());

    m_install->setInstallDocumentation(ui.download_doc->isChecked());

    if (ui.hostnameEdit->text().isEmpty()) {
        ui.hostnameEdit->setText("chakra-pc");
    }

    m_install->setHostname(ui.hostnameEdit->text());

    emit goToNextStep();

}

void SettingsPage::aboutToGoToPrevious()
{
    emit goToPreviousStep();
}

#include "settingspage.moc"
