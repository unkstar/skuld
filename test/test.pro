CONFIG += qt
TEMPLATE = app
TARGET=test


CONFIG += console

ROOT_DIR=..
include($$ROOT_DIR/include/common.pro)

win32 {
CONFIG(debug, debug|release) {
    LIBS += coroutined.lib
}

CONFIG(release, debug|release) {
    LIBS += coroutine.lib
}
}

unix {
  LIBS += -lcoroutine
}

# Input
HEADERS += $$files(test_*.h)

SOURCES += main.cpp
SOURCES += $$ROOT_DIR/3party/gtest/gtest-all.cc
