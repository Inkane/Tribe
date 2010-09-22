/***************************************************************************
 *
 *   Copyright (C) 2010 The Chakra Project
 *
 *   GPL3
 *
 ***************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

/* Defines the installation paths */
#define IMAGE_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/share/tribe/images"
#define SCRIPTS_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/share/tribe/scripts/"
#define CONFIG_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/share/tribe/config/"
#define STYLESHEET_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/share/tribe/style/tribe.qss"
#define TRIBE_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}"

/* Defines Tribe Build Revision (will appear on UI) */
#define TRIBE_BUILD_REVISION "source build/from git"

/* Defines Target Mount Point */
#define INSTALLATION_TARGET "/mnt/install.root"

/* Defines the version */
#define TRIBE_VERSION "1.0 alpha"

#endif /*CONFIG_H*/
