PROJECT(liblvm2-qt)

cmake_minimum_required(VERSION 2.8)

find_package(Qt4 REQUIRED)

set(QT_DONT_USE_QTGUI true)

add_definitions(${QT_DEFINITIONS})
include(${QT_USE_FILE})

add_subdirectory(lib)

