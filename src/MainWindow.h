/***************************************************************************
 *   Copyright (C) 2008, 2009  Dario Freddi <drf@chakra-project.org>       *
 *                 2008        Lukas Appelhans <l.appelhans@gmx.de>        *
 *                 2010        Drake Justice <djustice@chakra-project.org> *
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

#ifndef MAINWINDOW
#define MAINWINDOW

#include <QWidget>

#include <KMainWindow>

#include "ui_tribeBase.h"

class InstallationHandler;
class QMovie;

class MainWindow : public KMainWindow
{
    Q_OBJECT

public:
    enum InstallationStep {
        Welcome,
        ReleaseNotes,
        LicenseApproval,
        Language,
        Keymap,
        Preparation,
        Partitioning,
        Settings,
        ReadyToInstall,
        Installation,
        PreparingPartitions,
        InstallSystem,
        Configuration,
        CreateUser,
        RootPassword,
        Bootloader,
        FinishStep
    };

    enum StepStatus {
        InProgress,
        Done,
        ToDo
    };

public:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

protected:
    void closeEvent(QCloseEvent *evt);

private slots:
    void loadPage(InstallationStep page);
    void setInstallationStep(InstallationStep step, StepStatus status);
    void abortInstallation();
    void goToNextStep();
    void goToPreviousStep();
    void showProgressWidget();
    void deleteProgressWidget();

    void enableNextButton(bool enable);
    void enablePreviousButton(bool enable);

    void errorOccurred(const QString &error);

    void quitToChakra();
    void quitAndReboot();

    void setUpCleanupPage();

signals:
    void updateProgressWidget(int percentage);
    void setProgressWidgetText(const QString &);
    void readyToCreate();

private:
    Ui::TribeBase m_ui;
    InstallationStep m_currAction;
    InstallationHandler *m_install;

    QMovie *m_movie;
};

#endif /*MAINWINDOW*/
