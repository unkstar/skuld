#ifndef __COROUTINE_PROMISE_H__
#define __COROUTINE_PROMISE_H__
#pragma once

#include "deferred.h"
#include "deferred_p.h"
#include "deferred_base.h"
#include "deferred_ops.h"


class QObject;

template<typename Ty_>
struct is_void_type
{
  enum {
    value = 0
  };
};

template<>
struct is_void_type<void>
{
  enum {
    value = 1
  };
};

template<typename Ty_ = void>
class Promise
	: public DeferredBase
{
	public:

		Promise();
		Promise(bool s);
		explicit Promise(DeferredDataPtr d);
		Promise(const Deferred<Ty_> &d);
		Promise(const Promise &rhs);

    template<typename Other_>
		Promise(const Promise<Other_> &rhs)
      : DeferredBase(rhs)
    {
      STATIC_ASSERT(is_void_type<Other_>::value);
    }

		tribool reset();

		Ty_ result();

		Promise& operator=(const Promise& rhs)
		{
      this->assign(rhs);
			return *this;
		}

	template<typename T_, typename U_>
	friend Promise<void> operator|| (const Promise<T_> &lhs, const Promise<U_> &rhs);

	template<typename T_, typename U_>
	friend Promise<void> operator&& (const Promise<T_> &lhs, const Promise<U_> &rhs);

	template<typename T_>
	friend Promise<void> operator! (const Promise<T_> &in);

};


template<typename ResultType_ = void>
class AutoCancelPromise
	: public Promise<ResultType_>
{
public:
	AutoCancelPromise()
		: Promise<ResultType_>(), dismissed_(false)
	{
	}

	AutoCancelPromise(const Promise<ResultType_> &rhs)
		: Promise<ResultType_>(rhs), dismissed_(false)
	{
	}

	~AutoCancelPromise()
	{
		if(!dismissed_) {
			this->cancel();
		}
	}

	void dismiss() {dismissed_ = true;}

	AutoCancelPromise& operator=(const Promise<ResultType_>& rhs)
	{
		this->cancel();
		*(Promise<ResultType_>*)this = rhs;
		this->dismissed_ = false;
		return *this;
	}

	AutoCancelPromise& operator=(const AutoCancelPromise& rhs)
	{
		this->cancel();
		*(Promise<ResultType_>*)this = rhs;
		this->dismissed_ = rhs.dismissed_;
		rhs.dismiss();
		return *this;
	}

private:

	bool dismissed_;
};


template<typename T_, typename U_>
Promise<void> operator||(const Promise<T_> &lhs, const Promise<U_> &rhs)
{
	return Promise<void>(promiseOr(lhs.d_ptr, rhs.d_ptr));
}

template<typename T_, typename U_>
Promise<void> operator&&(const Promise<T_> &lhs, const Promise<U_> &rhs)
{
	return Promise<void>(promiseAnd(lhs.d_ptr, rhs.d_ptr));
}

template<typename T_>
Promise<void> operator!(const Promise<T_> &in)
{
	return Promise<void>(promiseNot(in.d_ptr));
}

template<typename T_, typename U_>
Promise<void> operator||(const Deferred<T_> &lhs, const Promise<U_> &rhs)
{
	return Promise<T_>(lhs) || rhs;
}

template<typename T_, typename U_>
Promise<void> operator&&(const Deferred<T_> &lhs, const Promise<U_> &rhs)
{
	return Promise<T_>(lhs) && rhs;
}

template<typename T_>
Promise<void> operator!(const Deferred<T_> &in)
{
	return !Promise<T_>(in);
}

template<typename T_, typename U_>
Promise<void> operator||(const Promise<T_> &lhs, const Deferred<U_> &rhs)
{
	return lhs || Promise<U_>(rhs);
}

template<typename T_, typename U_>
Promise<void> operator&&(const Promise<T_> &lhs, const Deferred<U_> &rhs)
{
	return lhs && Promise<U_>(rhs);
}

template<typename T_, typename U_>
Promise<void> operator||(const Deferred<T_> &lhs, const Deferred<U_> &rhs)
{
	return Promise<T_>(lhs) || Promise<U_>(rhs);
}

template<typename T_, typename U_>
Promise<void> operator&&(const Deferred<T_> &lhs, const Deferred<U_> &rhs)
{
	return Promise<T_>(lhs) && Promise<U_>(rhs);
}


//XXX:implement following shxt while I've higher sane value
//I hope Cthulhu can leave me alone...
//
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
