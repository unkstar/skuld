TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

HEADERS = global.h \
					coroutine.h \
					coroutine_p.h \
					promise.h \
					promise.inc \
					deferred.h \
					deferred_p.h \
					deferred_ops.h

SOURCES = main.cpp \
					coroutine.cpp \
					coroutine_p.cpp \
					deferred_p.cpp \
					md5.c
