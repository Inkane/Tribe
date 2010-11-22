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
	gui/*.cpp
	config/*.cpp
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

file(GLOB partitionmanagerprivate_UIFILES config/*.ui gui/*.ui)

kde4_add_ui_files(partitionmanagerprivate_SRCS ${partitionmanagerprivate_UIFILES})

kde4_add_kcfg_files(partitionmanagerprivate_SRCS config.kcfgc)

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

