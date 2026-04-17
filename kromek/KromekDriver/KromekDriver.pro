TEMPLATE = lib
CONFIG += staticlib
CONFIG -= app_bundle
CONFIG -= qt

# Make sure the correct arch variable is set (linux)
linux-g++:QMAKE_TARGET.arch = $$QMAKE_HOST.arch
linux-g++-32:QMAKE_TARGET.arch = x86
linux-g++-64:QMAKE_TARGET.arch = x86_64

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

INCLUDEPATH += include

#LIBS += -lSetupApi -lhid -lAdvapi32 -luser32
#Windows specific defines
win32:DEFINES += _WINDOWS
DEFINES += _UNICODE
win32:DEFINES += _USRDLL

#Cross platform defines
DEFINES += KROMEKDRIVER_EXPORTS

#Cross platform includes
SOURCES += \
    src/CriticalSection.cpp \
    src/D3DataProcessor.cpp \
    src/DeviceBase.cpp \
    src/DeviceMgr.cpp \
    src/Event.cpp \
    src/GR1.cpp \
    src/IntervalCountProcessor.cpp \
    src/K102.cpp \
    src/Lock.cpp \
    src/RadAngel.cpp \
    src/RollingQueue.cpp \
    src/SIGMA_25.cpp \
    src/SIGMA_50.cpp \
    src/stdafx.cpp \
    src/Thread.cpp \
    src/TN15.cpp \
    src/GR05.cpp \
    src/UNIBASE.cpp
    

HEADERS += \
    include/CriticalSection.h \
    include/D3DataProcessor.h \
    include/D3Structs.h \
    include/DeviceBase.h \
    include/DeviceMgr.h \
    include/Event.h \
    include/GR1.h \
    include/IDataInterface.h \
    include/IDataProcessor.h \
    include/IDevice.h \
    include/IntervalCountProcessor.h \
    include/K102.h \
    include/kromek.h \
    include/Lock.h \
    include/RadAngel.h \
    include/RollingQueue.h \
    include/SIGMA_25.h \
    include/SIGMA_50.h \
    include/stdafx.h \
    include/targetver.h \
    include/Thread.h \
    include/Time.h \
    include/TN15.h \
    include/GR05.h \
    include /UNIBASE.cpp \
    include/types.h \
    include/version.h \
    include/kromek_endian.h \
    include/ErrorCodes.h

# Windows specific files
win32:SOURCES += \
    src/DeviceEnumeratorWindows.cpp \
    src/USBSerialDataInterfaceWindows.cpp \
    src/HIDDataInterfaceWindows.cpp

win32:HEADERS += \
    include/DeviceEnumeratorWindows.h \
    include/USBSerialDataInterfaceWindows.h \
    include/HIDDataInterfaceWindows.h

# Linux specific files
unix:SOURCES += \
    src/DeviceEnumeratorLinux.cpp \
    src/USBKromekDataInterfaceLinux.cpp

unix:HEADERS += \
    include/DeviceEnumeratorLinux.h \
    include/USBKromekDataInterfaceLinux.h

include(deployment.pri)
qtcAddDeployment()


