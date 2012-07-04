TEMPLATE = lib
TARGET=coroutine
DEFINES += BUILD_DW_COROUTINE

ROOT_DIR=..
include($$ROOT_DIR/include/common.pro)

win32{

QMAKE_LFLAGS_DEBUG += /IMPLIB:../lib/$$(TARGET)d.lib
QMAKE_LFLAGS_RELEASE += /IMPLIB:../lib/$$(TARGET).lib

SOURCES += coroutine/coroutine_win32.cpp
SOURCES += async/async_jobpool_win32.cpp
SOURCES += async/asio_win32.cpp
HEADERS += async/asio_win32.h
SOURCES += misc/sync_win32.cpp

}

unix {

HEADERS += async/asio_linux.h

SOURCES += coroutine/coroutine_posix.cpp
SOURCES += async/async_jobpool_posix.cpp
SOURCES += async/asio_linux.cpp

#DEFINES += USE_QT_INFRASTRUCTURES
}



HEADERS += global.h
HEADERS += $$files(coroutine/*.h)
HEADERS += $$files(misc/*.h)
HEADERS += $$files(promise/*.h)


SOURCES += misc/sysinfo.cpp

SOURCES += $$files(promise/*.cpp)

SOURCES += coroutine/coroutine.cpp
SOURCES += coroutine/coroutine_p.cpp

HEADERS += async/async_jobpool.h
HEADERS += async/signal_promise.h
HEADERS += async/asio.h

SOURCES += async/async_jobpool.cpp
SOURCES += async/signal_promise.cpp
