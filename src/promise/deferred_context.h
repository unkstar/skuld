#ifndef __COROUTINE_DEFERRED_CONTEXT_H__
#define __COROUTINE_DEFERRED_CONTEXT_H__
#pragma once

#include "promise.h"


#define DEFERRED_OFFSETOF(t,m)   ((size_t)((char*)(&((t*)0)->m)))

#define PINPOINT_DEFERRED_CONTEXT(derived, ctxtype, ctxname) \
	ctxtype* toContext() {return &this->ctxname;} \
	static derived* fromContext(ctxtype* ctx) {return (derived*)(((char*)ctx) - DEFERRED_OFFSETOF(derived, ctxname));}

template<typename Ty_>
class DeferredContext
  : public DeferredBase,
	  public DeferredContextInterface
{
	public:
		DeferredContext();
		virtual ~DeferredContext() = 0;

		Promise<Ty_> promise();

		void resolve(const Ty_ &res);
		void reject();
    virtual tribool reset_me();
};

template<>
class DeferredContext<void>
  : public DeferredBase,
	  public DeferredContextInterface
{
	public:
		DeferredContext();
		virtual ~DeferredContext() = 0;

		Promise<void> promise()
		{
			return Promise<void>(d_ptr);
		}

		void resolve();
		void reject();
    virtual tribool reset_me();
};



#include "deferred_context.inc"

#endif //__COROUTINE_DEFERRED_CONTEXT_H__
