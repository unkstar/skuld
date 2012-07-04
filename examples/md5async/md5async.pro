TEMPLATE = app
TARGET=md5async


ROOT_DIR=../..
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
  #QMAKE_LFLAGS += -lcoroutine
  QMAKE_CXXFLAGS  += -g
}


# Input
HEADERS += md5.h \


SOURCES = main.cpp \
					md5.c
