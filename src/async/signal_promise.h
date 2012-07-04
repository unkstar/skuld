#ifndef __COROUTINE_SIGNAL_PROMISE_H__
#define __COROUTINE_SIGNAL_PROMISE_H__
#pragma once

#include <QObject>
#include "promise/promise.h"

class QObject;

typedef AutoCancelPromise<QVariant> SignalPromise;

DW_COROUTINE_EXPORT Promise<QVariant> signalPromise(QObject *src, const char *signal);

#endif //__COROUTINE_SIGNAL_PROMISE_H__
