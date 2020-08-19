CONFIG += qt

include($$PWD/../../../global.pri)

TEMPLATE = lib
DEFINES += HMDF_LIBRARY
TARGET = hmdf

CONFIG += c++11
CONFIG += static

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
    src/nefisseriesmetadata.cpp \
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
    include/nefisseriesmetadata.h \
    include/station.h \
    include/timepoint.h

NETCDFHOME=/usr/local/Cellar/netcdf/4.7.4
INCLUDEPATH += include
INCLUDEPATH += ../thirdparty/boost_1_73_0
INCLUDEPATH += ../thirdparty/boost_1_73_0
INCLUDEPATH += ../thirdparty
INCLUDEPATH += $$NETCDFHOME/include

LIBS += -L$$NETCDFHOME/lib -lnetcdf

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../thirdparty/nefis/release/ -lnefis
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../thirdparty/nefis/debug/ -lnefis
else:unix: LIBS += -L$$OUT_PWD/../thirdparty/nefis/ -lnefis

INCLUDEPATH += $$PWD/../thirdparty/nefis/include
DEPENDPATH += $$PWD/../thirdparty/nefis

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../thirdparty/nefis/release/libnefis.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../thirdparty/nefis/debug/libnefis.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../thirdparty/nefis/release/nefis.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../thirdparty/nefis/debug/nefis.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../thirdparty/nefis/libnefis.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../thirdparty/ezproj/src/release/ -lezproj
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../thirdparty/ezproj/src/debug/ -lezproj
else:unix: LIBS += -L$$OUT_PWD/../thirdparty/ezproj/src/ -lezproj

INCLUDEPATH += $$PWD/../thirdparty/ezproj/src
DEPENDPATH += $$PWD/../thirdparty/ezproj/src

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../thirdparty/ezproj/src/release/libezproj.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../thirdparty/ezproj/src/debug/libezproj.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../thirdparty/ezproj/src/release/ezproj.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../thirdparty/ezproj/src/debug/ezproj.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../thirdparty/ezproj/src/libezproj.a
