CONFIG -= qt

TEMPLATE = lib
DEFINES += HMDF_LIBRARY

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/date.cpp \
    src/hmdf.cpp \
    src/logging.cpp \
    src/station.cpp \
    src/timepoint.cpp

HEADERS += \
    include/date.h \
    include/hmdf.h \
    include/date.h \
    include/hmdf_global.h \
    include/logging.h \
    include/station.h \
    include/timepoint.h

NETCDFHOME=/usr/local/Cellar/netcdf/4.7.4
INCLUDEPATH += include
INCLUDEPATH += thirdparty/boost_1_73_0 thirdparty/ezproj/src
INCLUDEPATH += thirdparty
INCLUDEPATH += $$NETCDFHOME/include

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
