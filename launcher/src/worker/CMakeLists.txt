
INCLUDE_DIRECTORIES(
    ${QT_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set (TRIBELAUNCHER_HELPER_INSTALL_DIR ${LIB_INSTALL_DIR}/tribe)

qt4_add_dbus_adaptor(tribelauncherworker_adaptor_SRCS org.chakraproject.tribelauncherworker.xml Worker.h Worker)

SET(tribelauncherworker_SRCS 
    main.cpp
    Worker.cpp
)

automoc4_add_executable(tribelauncherworker
    ${tribelauncherworker_SRCS}
    ${tribelauncherworker_adaptor_SRCS}
)

TARGET_LINK_LIBRARIES(tribelauncherworker
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${POLKITQT_CORE_LIBRARY}
)

INSTALL(CODE "MESSAGE(\"Installing Tribelauncher worker...\")")
install(TARGETS tribelauncherworker DESTINATION ${TRIBELAUNCHER_HELPER_INSTALL_DIR})

#dbus_add_activation_system_service(org.chakraproject.tribelauncherworker.service.in)

install(FILES org.chakraproject.tribelauncherworker.conf DESTINATION /etc/dbus-1/system.d)
install(FILES org.chakraproject.tribelauncherworker.policy DESTINATION ${POLKITQT_POLICY_FILES_INSTALL_DIR})

