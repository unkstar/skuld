#ifndef __COROUTINE_DEFERRED_OPS_H__
#define __COROUTINE_DEFERRED_OPS_H__
#pragma once

class DeferredData;
class DeferredBase;

DW_COROUTINE_EXPORT DeferredDataPtr promiseNot(DeferredDataPtr in);
DW_COROUTINE_EXPORT DeferredDataPtr promiseOr(DeferredDataPtr lhs, DeferredDataPtr rhs);
DW_COROUTINE_EXPORT DeferredDataPtr promiseAnd(DeferredDataPtr lhs, DeferredDataPtr rhs);

#endif //__COROUTINE_DEFERRED_OPS_H__
