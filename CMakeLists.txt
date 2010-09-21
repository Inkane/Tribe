PROJECT(tribe)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(KDE4 REQUIRED)
find_package(KDE4Workspace 4.3.0 REQUIRED)

configure_file(config-tribe.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-tribe.h )

include (MacroLibrary)
include (FindPackageHandleStandardArgs)
include (KDE4Defaults)

find_package(MarbleWidget REQUIRED)
find_package(TribePartitionManager REQUIRED)
find_package(MSGFMT REQUIRED)

add_subdirectory(icons)
add_subdirectory(translations)

add_definitions (
    ${QT_DEFINITIONS} 
    ${QT_QTDBUS_DEFINITIONS} 
    ${KDE4_DEFINITIONS}
)

include_directories (
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${LIBMARBLEWIDGET_INCLUDE_DIR}
    ${TRIBEPARTITIONMANAGER_INCLUDE_DIR}
    ${KDE4WORKSPACE_INCLUDE_DIR}
)

SET(tribe_SRCS
    src/main.cpp
    src/MainWindow.cpp
    src/InstallationHandler.cpp
    src/AbstractPage.cpp
    src/CrashHandler.cpp
    src/Screenshots.h
    src/PMHandler.cpp
)

set(ksmserver_xml ${DBUS_INTERFACES_INSTALL_DIR}/org.kde.KSMServerInterface.xml)

qt4_add_dbus_interface(tribe_SRCS ${ksmserver_xml} ksmserver_interface )

SET(tribe_PAGES
    src/pages/IntroPage.cpp
    src/pages/ReleaseNotesPage.cpp
    src/pages/PartitioningPage.cpp
    src/pages/ProgressWidget.cpp
    src/pages/UserCreationPage.cpp
    src/pages/InstallationPage.cpp
    src/pages/ReadyInstallPage.cpp
    src/pages/BootloaderPage.cpp
    src/pages/FinishPage.cpp
    src/pages/LicensePage.cpp
    src/pages/SettingsPage.cpp
    src/pages/LocaleSelectorPage.cpp
)

SET(tribe_UIS
    ui/tribeBase.ui
    ui/tribeIntro.ui
    ui/tribeReleaseNotes.ui
    ui/tribePartition.ui
    ui/tribeProgressWidget.ui
    ui/tribeUserCreation.ui
    ui/tribeInstallation.ui
    ui/tribeReadyInstall.ui
    ui/tribeBootloader.ui
    ui/tribeFinish.ui
    ui/tribeLicense.ui
    ui/tribeSettings.ui
    ui/tribeLocale.ui
    ui/crashWindow.ui
)

SET(tribe_IMAGES
    ui/images/background.svg
    ui/images/sidebar.svg
)

SET(tribe_RESOUCES
    ui/tribe.qrc
)

SET(tribe_STYLESHEET
    ui/tribe.qss
)

SET(tribe_MUSIC
    music/oobe.ogg
)

QT4_ADD_RESOURCES (qtsourceview_RC_SRCS  ${tribe_RESOUCES})

kde4_add_ui_files(tribe_SRCS ${tribe_UIS})

kde4_add_executable(tribe
    ${tribe_SRCS}
    ${tribe_PAGES}
    ${tribe_WIDGETS}
    ${qtsourceview_RC_SRCS}
)

TARGET_LINK_LIBRARIES(tribe
    kworkspace
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${KDE4_KUTILS_LIBRARY}
    ${KDE4_KIO_LIBRARY}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_PHONON_LIBRARY}
    ${LIBMARBLEWIDGET_LIBRARY}
    ${TRIBEPARTITIONMANAGER_LIBRARY}
    ${KDE4WORKSPACE_KWORKSPACE_LIBS}
    -lsolidcontrol
)

CONFIGURE_FILE(
       "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/cmake_uninstall.cmake.in"
       "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
       IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

file(GLOB POSTINSTALL_FUNCTIONS
    etc/postinstall-functions/*
)

INSTALL(CODE "MESSAGE(\"Installing Tribe executable...\")")
INSTALL(TARGETS tribe DESTINATION bin)

INSTALL(CODE "MESSAGE(\"Installing Tribe scripts...\")")
INSTALL(FILES etc/postinstall.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/scripts)
INSTALL(FILES etc/postremove.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/scripts)
INSTALL(FILES ${POSTINSTALL_FUNCTIONS} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/scripts)
INSTALL(FILES etc/prepare-swap.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/scripts)

INSTALL(CODE "MESSAGE(\"Installing Tribe configuration files...\")")
INSTALL(FILES etc/postinstall.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/config)
INSTALL(FILES etc/release_notes.txt DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/config)

INSTALL(CODE "MESSAGE(\"Installing Tribe service files...\")")
INSTALL(FILES etc/tribe.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)

INSTALL(CODE "MESSAGE(\"\")")

INSTALL(CODE "MESSAGE(\"Installing Tribe Multimedia Components...\")")
#INSTALL(FILES ${tribe_IMAGES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/images)
INSTALL(FILES ${tribe_STYLESHEET} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/style)
#INSTALL(FILES ${tribe_MUSIC} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/music)
#Don't force the install prefix
INSTALL(FILES etc/cities.kml DESTINATION ${DATA_INSTALL_DIR}/marble/data/placemarks)
INSTALL(FILES etc/timezones DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/config)
INSTALL(FILES etc/all_locales DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/config)
INSTALL(FILES etc/all_kde_langpacks DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tribe/config)
