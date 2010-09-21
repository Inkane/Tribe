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

#include "TribeLauncher.h"

#include <KUniqueApplication>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

int main(int argc, char **argv)
{
    KLocale::setMainCatalog("tribe");

    KAboutData aboutData("tribelauncher", 0, ki18n("Tribe Launcher"),
                         "1.0", ki18n("An auto updater and launcher for Tribe"), KAboutData::License_GPL,
                         ki18n("(c) 2008 the Chakra Development Team"), ki18n("team@chakra-project.org"), "http://chakra-project.org");

    aboutData.addAuthor(ki18n("Dario Freddi"), ki18n("Developer"), "drf@chakra-project.org", "http://drfav.wordpress.com");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app(true);

    app.setWindowIcon(KIcon("tribe"));

    KDialog *dlg = new KDialog(0, Qt::FramelessWindowHint);
    dlg->setButtons(0);

    TribeLauncher *launcher = new TribeLauncher;
    dlg->setMainWidget(launcher);

    dlg->show();

    return app.exec();
}
