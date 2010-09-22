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

#include <QFile>
#include <QItemSelectionModel>
#include <QMouseEvent>

#include <KIcon>
#include <KLocale>
#include <KDebug>
#include <KMessageBox>

#include <marble/MarbleWidget.h>
#include <marble/MarbleModel.h>

#include "LocaleSelectorPage.h"

#include "../InstallationHandler.h"

#include <config-tribe.h>

using namespace Marble;

LocaleSelectorPage::LocaleSelectorPage(QWidget *parent)
        : AbstractPage(parent),
        m_install(InstallationHandler::instance())
{
}

LocaleSelectorPage::~LocaleSelectorPage()
{
}

void LocaleSelectorPage::createWidget()
{
    setupUi(this);

    zoomInButton->setIcon(KIcon("zoom-in"));
    zoomOutButton->setIcon(KIcon("zoom-out"));

    marble->installEventFilter(this);
    marble->setMapThemeId("earth/srtm/srtm.dgml");

    marble->setShowCities(false);

    // disable overlays, crosshair and the grid
    marble->setShowOverviewMap(false);
    marble->setShowScaleBar(false);
    marble->setShowCompass(false);
    marble->setShowCrosshairs(false);
    marble->setShowGrid(false);

    marble->addPlacemarkFile("cities.kml");

    /** Read our locales file **/
    QFile fp(QString(CONFIG_INSTALL_PATH) + "timezones");

    if (!fp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kDebug() << "Failed to open file";
    }

    QTextStream in(&fp);

    locales.clear();

    /* Get the file contents */

    while (!in.atEnd()) {
        QString line(in.readLine());
        QStringList split = line.split(':');
        locales.append(split);
    }

    fp.close();

    fp.setFileName(QString(CONFIG_INSTALL_PATH) + "all_kde_langpacks");

    if (!fp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kDebug() << "Failed to open file";
    }

    //QTextStream in(&fp);

    /* Get the file contents */

    while (!in.atEnd()) {
        QString line(in.readLine());
        QStringList split = line.split(":");
        m_allKDELangs.insert(split.first(), split.last());
    }

    fp.close();

    kDebug() << locales;
    QStringList keys;
    foreach (const QStringList &l, locales) {
        keys << l.first();
    }
    keys.sort();
    foreach(const QString &string, keys) {
        timezoneCombo->addItem(string);
    }

    locationsSearch->hide();
    locationsView->hide();

    timezoneChanged(timezoneCombo->currentIndex());

    connect(zoomInButton, SIGNAL(clicked()), marble, SLOT(zoomIn()));
    connect(zoomOutButton, SIGNAL(clicked()), marble, SLOT(zoomOut()));
    connect(zoomSlider, SIGNAL(valueChanged(int)), SLOT(zoom(int)));
    connect(marble, SIGNAL(zoomChanged(int)), this, SLOT(zoomChanged(int)));
    connect(timezoneCombo, SIGNAL(currentIndexChanged(int)), SLOT(timezoneChanged(int)));
    connect(showLocalesCheck, SIGNAL(stateChanged(int)), SLOT(updateLocales()));
    connect(showKDELangsCheck, SIGNAL(stateChanged(int)), SLOT(updateLocales()));

    zoom(60);
}

void LocaleSelectorPage::zoom(int value)
{
    marble->zoomView(value * 20);
}

bool LocaleSelectorPage::eventFilter(QObject * object, QEvent * event)
{
    if (object == marble && event->type() == QEvent::MouseButtonPress) {
        QVector<QModelIndex> indexes = marble->model()->whichFeatureAt(marble->mapFromGlobal(QCursor::pos()));
        if (!indexes.isEmpty()) {
            timezoneCombo->setCurrentIndex(timezoneCombo->findText(indexes.first().data(Qt::DisplayRole).toString().replace(' ', '_'), Qt::MatchContains));
            return true;
        }
    }
    return false;
}

void LocaleSelectorPage::timezoneChanged(int index)
{
    if (!showLocalesCheck->isChecked() || !showKDELangsCheck->isChecked()) {
        QString time = timezoneCombo->itemText(index);
        QList<QStringList>::const_iterator it;
        for (it = locales.constBegin(); it != locales.constEnd(); ++it) {
            if ((*it).first() == time) {
                if (!showLocalesCheck->isChecked()) {
                    localeCombo->clear();
                    foreach(const QString &str, (*it).at(1).split(',')) {
                        localeCombo->addItem(str);
                    }
                }
                if (!showKDELangsCheck->isChecked()) {
                    kdeLanguageCombo->clear();
                    foreach(const QString &str, (*it).at(2).split(',')) {
                        kdeLanguageCombo->addItem(m_allKDELangs.value(str));
                    }
                }
            }
        }
    }
}

void LocaleSelectorPage::updateLocales()
{
    if (showLocalesCheck->isChecked()) {
        if (m_allLocales.isEmpty()) {
            QFile fp(QString(CONFIG_INSTALL_PATH) + "all_locales");

            if (!fp.open(QIODevice::ReadOnly | QIODevice::Text)) {
                kDebug() << "Failed to open file";
            }

            QTextStream in(&fp);

            /* Get the file contents */

            while (!in.atEnd()) {
                QString line(in.readLine());
                m_allLocales.append(line);
            }

            fp.close();
        }
        QString current = localeCombo->currentText();
        localeCombo->clear();
        foreach (const QString &loc, m_allLocales) {
            localeCombo->addItem(loc);
        }
        if (localeCombo->findText(current) != -1) {
            localeCombo->setCurrentIndex(localeCombo->findText(current));
        }
    }
    if (showKDELangsCheck->isChecked()) {
        QString current = kdeLanguageCombo->currentText();
        kdeLanguageCombo->clear();
        QStringList values = m_allKDELangs.values();
        values.sort();
        foreach (const QString &loc, values) {
            kdeLanguageCombo->addItem(loc);
        }
        if (kdeLanguageCombo->findText(current) != -1) {
            kdeLanguageCombo->setCurrentIndex(kdeLanguageCombo->findText(current));
        }
    }
    if (!showKDELangsCheck->isChecked() || !showLocalesCheck->isChecked()) {
        timezoneChanged(timezoneCombo->currentIndex());
    }
}

void LocaleSelectorPage::zoomChanged(int)
{
    disconnect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(zoom(int)));
    zoomSlider->setValue(marble->zoom() / 20);
    connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(zoom(int)));
}

void LocaleSelectorPage::aboutToGoToNext()
{
    if (timezoneCombo->currentText().isEmpty()) {
        bool retbool;
        KDialog *dialog = new KDialog(this, Qt::FramelessWindowHint);
        dialog->setButtons(KDialog::Ok);
        dialog->setModal(true);

        KMessageBox::createKMessageBox(dialog, QMessageBox::Warning,
                                        i18n("You need to select a timezone"),
                                        QStringList(), QString(), &retbool, KMessageBox::Notify);
        return;
    }
    m_install->setTimezone(timezoneCombo->currentText());
    m_install->setKDELangPack(m_allKDELangs.key(kdeLanguageCombo->currentText()));
    if (localeCombo->currentText().contains("utf-8", Qt::CaseInsensitive)) {
        QStringList localeSplit = localeCombo->currentText().split('.');
        localeSplit.last().replace('-', QString());
        localeSplit.last() = localeSplit.last().toLower();
        m_install->setLocale(localeSplit.join("."));
    } else {
        m_install->setLocale(localeCombo->currentText());
    }
    emit goToNextStep();
}

void LocaleSelectorPage::aboutToGoToPrevious()
{
    emit goToPreviousStep();
}

#include "LocaleSelectorPage.moc"
