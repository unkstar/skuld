TEMPLATE = app
TARGET=md5async


ROOT_DIR=../..
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
  QMAKE_CXXFLAGS  += -g
}


# Input
HEADERS += md5.h \


SOURCES = main.cpp \
					md5.c
