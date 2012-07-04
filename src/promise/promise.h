#ifndef __COROUTINE_PROMISE_H__
#define __COROUTINE_PROMISE_H__
#pragma once

#include "deferred.h"
#include "deferred_p.h"
#include "deferred_ops.h"


class DeferredData;

template<typename Ty_ = void>
class Promise
{
	public:
		typedef boost::function<void()> promise_callback_type;
		typedef boost::function<void(tribool)> promise_always_type;

		Promise();
		explicit Promise(DeferredData *d);
		Promise(const Promise &rhs);
		~Promise();

		void cancel();

		size_t done(const promise_callback_type &func);
		size_t fail(const promise_callback_type &func);
		size_t always(const promise_always_type &func);

		void removeDone(size_t handle);
		void removeFail(size_t handle);
		void removeAlways(size_t handle);

		Ty_ result();


		bool isRejected();
		bool isResolved();

		tribool state();

		Promise& operator=(const Promise& rhs)
		{
			if(rhs.d_ptr == this->d_ptr) {
				return *this;
			}
			if(this->d_ptr) {
				d_ptr->Release();
			}
			d_ptr = rhs.d_ptr;
			if(d_ptr) {
				d_ptr->AddRef();
			}
			return *this;
		}

	template<typename T_, typename U_>
	friend Promise<void> operator|| (Promise<T_> &lhs, Promise<U_> &rhs);

	template<typename T_, typename U_>
	friend Promise<void> operator&& (Promise<T_> &lhs, Promise<U_> &rhs);

	template<typename T_>
	friend Promise<void> operator! (Promise<T_> &in);

	private:
		DeferredData *d_ptr;
};

template<typename T_, typename U_>
Promise<void> operator||(Promise<T_> &lhs, Promise<U_> &rhs)
{
	return Promise<void>(new OrOpDeferredData(lhs.d_ptr, rhs.d_ptr));
}

template<typename T_, typename U_>
Promise<void> operator&&(Promise<T_> &lhs, Promise<U_> &rhs)
{
	return Promise<void>(new AndOpDeferredData(lhs.d_ptr, rhs.d_ptr));
}

template<typename T_>
Promise<void> operator!(Promise<T_> &in)
{
	return Promise<void>(new NotOpDeferredData(in.d_ptr));
}

//XXX:implement following shxt while I've clear mind
//revert from determinate state into indeterminate state break
//the resolve/reject once rule...
template<typename T_>
Promise<void> operator==(Promise<T_> &lhs, bool rhs);

template<typename T_>
Promise<void> operator==(bool lhs, Promise<T_> &rhs);

template<typename T_>
Promise<void> operator!=(Promise<T_> &lhs, bool rhs);

template<typename T_>
Promise<void> operator!=(bool lhs, Promise<T_> &rhs);

template<typename T_>
Promise<void> operator==(Promise<T_> &lhs, boost::logic::detail::indeterminate_t);

template<typename T_>
Promise<void> operator==(boost::logic::detail::indeterminate_t, Promise<T_> &rhs);

template<typename T_>
Promise<void> operator!=(Promise<T_> &lhs, boost::logic::detail::indeterminate_t);

template<typename T_>
Promise<void> operator!=(boost::logic::detail::indeterminate_t, Promise<T_> &rhs);




#include "promise.inc"

#endif //__COROUTINE_PROMISE_H__
