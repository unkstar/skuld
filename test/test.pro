CONFIG += qt
TEMPLATE = app
TARGET=test


CONFIG += console

ROOT_DIR=..
include($$ROOT_DIR/include/common.pro)

win32 {
CONFIG(debug, debug|release) {
    LIBS += skuldd.lib
}

CONFIG(release, debug|release) {
    LIBS += skuld.lib
}
}

unix {
  LIBS += -lskuld
}

# Input
HEADERS += $$files(test_*.h)

SOURCES += main.cpp
SOURCES += $$ROOT_DIR/3party/gtest/gtest-all.cc



runme.files                =
runme.path                 = .
!isEmpty(DESTDIR): runme.commands = cd ./$(DESTDIR) &&
macx:      runme.commands += ./$$TARGET.app/Contents/MacOS/$$TARGET
else:unix: runme.commands += ./$$TARGET
else:      runme.commands += $$TARGET

INSTALLS += runme
