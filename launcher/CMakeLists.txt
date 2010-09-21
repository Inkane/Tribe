PROJECT(tribelauncher)

add_subdirectory(src)

include_directories (
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_BINARY_DIR} 
    ${KDE4_INCLUDES} 
)

add_definitions (
    ${QT_DEFINITIONS} 
    ${QT_QTDBUS_DEFINITIONS} 
    ${KDE4_DEFINITIONS}
)

SET(tribelauncher_SRCS
    src/main.cpp
    src/TribeLauncher.cpp
)

SET(tribelauncher_RESOUCES
    ui/tribelauncher.qrc
)

QT4_ADD_RESOURCES (qtsourceview_RC_SRCS  ${tribelauncher_RESOUCES})

SET(tribelauncher_UIS
    ui/gui.ui
)

kde4_add_ui_files(tribelauncher_SRCS ${tribelauncher_UIS})

kde4_add_executable(tribelauncher
    ${tribelauncher_SRCS}
    ${qtsourceview_RC_SRCS}
)

TARGET_LINK_LIBRARIES(tribelauncher
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_SOLID_LIBRARY}
    ${POLKITQT_LIBRARIES}
    ${AQPM_LIBRARY}
)

INSTALL(CODE "MESSAGE(\"Installing Tribe Launcher...\")")
INSTALL(TARGETS tribelauncher DESTINATION bin)
