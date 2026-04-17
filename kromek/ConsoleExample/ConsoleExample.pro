TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += console.c

INCLUDEPATH += $$PWD/../SpectrometerDriver
DEPENDPATH += $$PWD/../SpectrometerDriver

# Make sure the correct arch variable is set (linux)
linux-g++:QMAKE_TARGET.arch = $$QMAKE_HOST.arch
linux-g++-32:QMAKE_TARGET.arch = x86
linux-g++-64:QMAKE_TARGET.arch = x86_64

contains(QMAKE_TARGET.arch, x86_64): {
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/win64/release -lSpectrometerDriver
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/win64/debug -lSpectrometerDriver

unix:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/linux64/release -lSpectrometerDriver
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/linux64/debug -lSpectrometerDriver
} else {
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/win32/release -lSpectrometerDriver
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/win32/debug -lSpectrometerDriver

unix:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/linux32/release -lSpectrometerDriver
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/linux32/debug -lSpectrometerDriver
}
