INCLUDEPATH += .

INCLUDEPATH += $$ROOT_DIR/include
INCLUDEPATH += $$ROOT_DIR/3party

CONFIG += debug_and_release

CONFIG(debug, debug|release) {
  DESTDIR = $$ROOT_DIR/bin/debug
  OBJECTS_DIR = $$ROOT_DIR/tmp/debug/$$TARGET
} else {
  DESTDIR = $$ROOT_DIR/bin/release
  OBJECTS_DIR = $$ROOT_DIR/tmp/release/$$TARGET
}


#PRECOMPILED_HEADER = stable.h

QMAKE_LIBDIR += $$ROOT_DIR/lib
QMAKE_LIBDIR += $$DESTDIR

win32{
DEFINES += UNICODE \
           NOMINMAX \
           WIN32_LEAN_AND_MEAN

QMAKE_CXXFLAGS += /EHsc
QMAKE_LFLAGS_RELEASE += /DEBUG
CharacterSet = 1
}

unix {
system(mkdir -p $$DESTDIR)
system(mkdir -p $$OBJECTS_DIR)
}
