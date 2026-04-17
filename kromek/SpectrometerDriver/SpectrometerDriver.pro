TARGET = SpectrometerDriver
TEMPLATE = lib

#CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

# Make sure the correct arch variable is set (linux)
linux-g++:QMAKE_TARGET.arch = $$QMAKE_HOST.arch
linux-g++-32:QMAKE_TARGET.arch = x86
linux-g++-64:QMAKE_TARGET.arch = x86_64
*-g++*:QMAKE_CXXFLAGS += -Wno-enum-compare

# Output directory
contains(QMAKE_TARGET.arch, x86_64): {
win32:CONFIG(debug, debug|release): DESTDIR = $$PWD/../libs/win64/debug
else:win32:CONFIG(release, debug|release): DESTDIR = $$PWD/../libs/win64/release

unix:CONFIG(release, debug|release): DESTDIR = $$PWD/../libs/linux64/release
else:unix:CONFIG(debug, debug|release): DESTDIR = $$PWD/../libs/linux64/debug
} else {
win32:CONFIG(debug, debug|release): DESTDIR = $$PWD/../libs/win32/debug
else:win32:CONFIG(release, debug|release): DESTDIR = $$PWD/../libs/win32/release

unix:CONFIG(release, debug|release): DESTDIR = $$PWD/../libs/linux32/release
else:unix:CONFIG(debug, debug|release): DESTDIR = $$PWD/../libs/linux32/debug
}

win32:DEFINES += _WINDOWS
DEFINES += _UNICODE
DEFINES += _USRDLL
DEFINES += USBSPECTROMETERDLL_EXPORTS \


SOURCES += \
    Detector.cpp \
    DriverMgr.cpp \
    SpectrometerDriver.cpp \
    stdafx.cpp

win32:SOURCES += dllmain.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    Detector.h \
    devices.h \
    DriverMgr.h \
    SpectrometerData.h \
    SpectrometerDriver.h \
    stdafx.h \
    targetver.h \
    version.h


contains(QMAKE_TARGET.arch, x86_64): {
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/win64/release/ -lKromekDriver
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/win64/debug -lKromekDriver

unix:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/linux64/release/ -lKromekDriver
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/linux64/debug -lKromekDriver
} else {
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/win32/release/ -lKromekDriver
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/win32/debug -lKromekDriver

unix:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/linux32/release/ -lKromekDriver
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/linux32/debug -lKromekDriver
}

win32:LIBS += -lSetupApi -lhid -lAdvapi32 -luser32
unix:LIBS += -ludev
INCLUDEPATH += $$PWD/../KromekDriver/include
DEPENDPATH += $$PWD/../KromekDriver/include
