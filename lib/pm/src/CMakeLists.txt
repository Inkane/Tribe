# Copyright (C) 2008 by Volker Lanz <vl@fidra.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

############################################

file(GLOB partitionmanagerprivate_SRCS
	backend/*.cpp
	core/*.cpp
	util/*.cpp
	ops/*.cpp
	jobs/*.cpp
	fs/*.cpp
)

file(GLOB partitionmanagerprivate_core_HDRS
    core/*.h
)
file(GLOB partitionmanagerprivate_fs_HDRS
    fs/*.h
)
file(GLOB partitionmanagerprivate_util_HDRS
    util/*.h
)
file(GLOB partitionmanagerprivate_ops_HDRS
    ops/*.h
)

kde4_add_library(tribepartitionmanager SHARED ${partitionmanagerprivate_SRCS})

target_link_libraries(tribepartitionmanager libfatlabel ${KDE4_KDECORE_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KIO_LIBS}
                      ${UUID_LIBRARIES} ${BLKID_LIBRARIES} ${LIBATASMART_LIBRARIES} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS tribepartitionmanager ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${partitionmanagerprivate_core_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/tribepartitionmanager/core)
install(FILES ${partitionmanagerprivate_fs_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/tribepartitionmanager/fs)
install(FILES ${partitionmanagerprivate_util_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/tribepartitionmanager/util)
install(FILES ${partitionmanagerprivate_ops_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/tribepartitionmanager/ops)

############################################

add_subdirectory(plugins)

# file(GLOB partitionmanager-bin_SRCS	main.cpp)
# 
# kde4_add_app_icon(partitionmanager-bin_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-apps-partitionmanager.png")
# 
# kde4_add_executable(partitionmanager-bin ${partitionmanager-bin_SRCS})
# 
# target_link_libraries(partitionmanager-bin partitionmanagerprivate ${KDE4_KDEUI_LIBS})
# 
# if(UNIX)
# 	add_custom_target(partitionmanager ALL
# 		COMMAND sed -e 's,@INSTALL_PATH@,${BIN_INSTALL_DIR},g' < ${CMAKE_CURRENT_SOURCE_DIR}/partitionmanager.in >
#${CMAKE_CURRENT_BINARY_DIR}/partitionmanager
# 	)
# 	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/partitionmanager DESTINATION ${BIN_INSTALL_DIR})
# endif(UNIX)
# install(TARGETS partitionmanager-bin ${INSTALL_TARGETS_DEFAULT_ARGS})
# install(FILES gui/partitionmanagerui.rc DESTINATION ${DATA_INSTALL_DIR}/partitionmanager)
# install(FILES partitionmanager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
# 
# ############################################
# 
# option(PARTMAN_KPART "Build a kpart for KDE Partition Manager" OFF)
# 
# if(PARTMAN_KPART)
# 	file(GLOB partitionmanagerpart_SRCS kpart/partitionmanagerpart.cpp)
# 
# 	kde4_add_plugin(partitionmanagerpart ${partitionmanagerpart_SRCS})
# 
# 	target_link_libraries(partitionmanagerpart
# 		${KDE4_KPARTS_LIBS}
# 		${KDE4_KDECORE_LIBS}
# 		partitionmanagerprivate
# 	)
# 
# 	install(TARGETS partitionmanagerpart DESTINATION ${PLUGIN_INSTALL_DIR})
# 	install(FILES kpart/partitionmanagerpart.desktop DESTINATION ${SERVICES_INSTALL_DIR})
# 	install(FILES kpart/partitionmanagerpart.rc DESTINATION ${DATA_INSTALL_DIR}/partitionmanagerpart)
# 
# 	option(PARTMAN_KPART_TEST "Build a kpart test application for KDE Partition Manager" OFF)
# 
# 	if(PARTMAN_KPART_TEST)
# 		add_subdirectory(kpart/test)
# 	endif(PARTMAN_KPART_TEST)
# 
# endif(PARTMAN_KPART)
# 
# ############################################
# 
# option(PARTMAN_KCM "Build a kcm for KDE Partition Manager" OFF)
# 
# if(PARTMAN_KCM)
# 	file(GLOB partitionmanagerkcm_SRCS kcm/partitionmanagerkcm.cpp)
# 
# 	kde4_add_plugin(kcm_partitionmanager ${partitionmanagerkcm_SRCS})
# 
# 	target_link_libraries(kcm_partitionmanager
# 		${KDE4_KUTILS_LIBS}
# 		${KDE4_KDEUI_LIBS}
# 		partitionmanagerprivate
# 	)
# 
# 	install(TARGETS kcm_partitionmanager DESTINATION ${PLUGIN_INSTALL_DIR})
# 	install(FILES kcm/kcm_partitionmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR})
# 
# endif(PARTMAN_KCM)
