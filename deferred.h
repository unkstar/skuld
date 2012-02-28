#ifndef __COROUTINE_DEFERRED_H__
#define __COROUTINE_DEFERRED_H__
#pragma once

#include "promise.h"


class DeferredData;

template<typename Ty_ = void>
class Deferred
{
	public:
		typedef boost::function<void()> promise_callback_type;
		typedef boost::function<void(tribool)> promise_always_type;
		typedef DeferredData::cancel_routine cancel_routine;

		Deferred();
		explicit Deferred(tribool s);
		Deferred(const Deferred &rhs);
		~Deferred();

		template<typename U>
		void setResult(const U &val) {
			d_ptr->setResult<Ty_>(val);
		}

		Promise<Ty_> promise() {
			return Promise<Ty_>(d_ptr);
		}

		void setContext(void * ctx, cancel_routine cancel) {
			d_ptr->setContext(ctx, cancel);
		}

		void* context() {
			return d_ptr->context();
		}

		void resolve();
		void reject();
		void cancel();
		void detach() {d_ptr->detach();}

		size_t done(const promise_callback_type &func);
		size_t fail(const promise_callback_type &func);
		size_t always(const promise_always_type &func);

		void removeDone(size_t handle);
		void removeFail(size_t handle);
		void removeAlways(size_t handle);

		bool isRejected();
		bool isResolved();

		tribool state();


	private:
		DeferredData *d_ptr;
};

#include "deferred.inc"

#endif //__COROUTINE_DEFERRED_H__
