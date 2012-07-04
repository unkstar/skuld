#ifndef __COROUTINE_DEFERRED_BASE_H__
#define __COROUTINE_DEFERRED_BASE_H__
#pragma once

#include "../global.h"
#include <boost/function.hpp>

class DeferredData;
class DeferredContextInterface;
typedef boost::shared_ptr<DeferredData> DeferredDataPtr;


//deferred/promise/deferred context base class
class DW_COROUTINE_EXPORT DeferredBase
{
	public:
		typedef boost::function<void()> promise_callback_type;
		typedef boost::function<void(tribool)> promise_always_type;

		DeferredBase();
		explicit DeferredBase(DeferredDataPtr d);
		explicit DeferredBase(DeferredContextInterface *ctx);
		DeferredBase(const DeferredBase &rhs);
		virtual ~DeferredBase() = 0;

		void cancel();

		size_t done(const promise_callback_type &func);
		size_t fail(const promise_callback_type &func);
		size_t always(const promise_always_type &func);

		void removeDone(size_t handle);
		void removeFail(size_t handle);
		void removeAlways(size_t handle);

		bool isRejected();
		bool isResolved();

		bool isActivating();

		tribool state() const;

		void setContext(DeferredContextInterface *ctx);


	protected:

    void assign(const DeferredBase &rhs);

		void resetState(tribool s);
		DeferredDataPtr d_ptr;
};


class DW_COROUTINE_EXPORT DeferredContextInterface
{
	public:
		DeferredContextInterface();
		virtual ~DeferredContextInterface() = 0;
		virtual void cancel_me() = 0;
		virtual tribool reset_me() = 0;
};

#endif //__COROUTINE_DEFERRED_BASE_H__
