set(LVM_SRCS lvm.cpp vg.cpp pv.cpp lv.cpp)
set(LVM_HDRS lvm.h vg.h pv.h lv.h)

qt4_wrap_cpp(LVM_MOC ${LVM_HDRS})

add_library(lvm2-qt SHARED ${LVM_SRCS} ${LVM_MOC})
#TODO write a FindLvm2app cmake module
target_link_libraries(lvm2-qt lvm2app ${QT_LIBRARIES})

install(TARGETS lvm2-qt LIBRARY DESTINATION lib)

install(DIRECTORY ./
        DESTINATION include/lvm2-qt
        FILES_MATCHING PATTERN "*.h")