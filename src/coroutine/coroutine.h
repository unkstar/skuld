#ifndef __COROUTINE_COROUTINE_H__
#define __COROUTINE_COROUTINE_H__

#pragma once

#include <boost/bind.hpp>
#include "promise/promise.h"

typedef boost::function<void()> coroutine_proc_type;


class CoroutineData;
typedef boost::shared_ptr<CoroutineData> CoroutineDataPtr;

template<typename Ty_>
class CoroutineT;

#include "coroutine_p.h"

class DW_COROUTINE_EXPORT CancelAsyncException : public std::exception
{
};



/*****************************************************************************
 * Under Win32:
 * This coroutine is build on top of Windows fiber, with two restriction:
 * 1) Add thread affinity to fiber, we don't allow fiber run on
 *		other thread rather than the creation one.
 * 2) Make it asymmetric, Windows fibers are symmetric,
 *		but we prefer LUA like asymmetric implementation
 * Under Unix:
 * ucontext is used to implement fiber, other constrains are the same
 *****************************************************************************/
class DW_COROUTINE_EXPORT Coroutine
{
	public:
		explicit Coroutine(const coroutine_proc_type &func);
		explicit Coroutine(CoroutineDataPtr d);
		//copy constructable
		Coroutine(const Coroutine &rhs);
		~Coroutine();

		//chainable
		Coroutine& run();
		Coroutine& yield();
		bool isCancelled();

		Promise<> promise();

    template<typename Ret_>
      friend struct ResultHelper;

    template<typename func_type>
      friend CoroutineT<typename func_type::result_type> Async__(const func_type &func);

	protected:
    ThreadId threadId();

		CoroutineDataPtr d_ptr;
};

template<typename Ty_>
class CoroutineT : public Coroutine
{
  public:
    CoroutineT(const CoroutineT &rhs)
      : Coroutine(rhs)
    {
    }

    Promise<Ty_> promise() {
      return Promise<Ty_>(d_ptr->d_ptr);  //WTF
    }

    template<typename func_type>
      friend CoroutineT<typename func_type::result_type> Async__(const func_type &func);

  private:
    explicit CoroutineT(CoroutineDataPtr d)
      : Coroutine(d)
    {
    }
};

DW_COROUTINE_EXPORT Coroutine GetCurrentCoroutine();


//DW_COROUTINE_EXPORT Coroutine Async__(const coroutine_proc_type &func);

//#define async(fna) Async__(boost::bind fna).run()

//use async like this:
//async((foo, arg1, arg2));

//also, chainable
template<typename Ty_>
Promise<Ty_> await(Promise<Ty_> pro)
{
	if(indeterminate(pro.reset())) {
		Coroutine cur = GetCurrentCoroutine();
		size_t h = pro.always(boost::bind(&Coroutine::run, cur));
		cur.yield();
		pro.removeAlways(h);
		if(cur.isCancelled()) {
			pro.cancel();
			throw CancelAsyncException();
		}
	}
	return pro;
}

#define async(fna) Async__(boost::bind fna)

template<typename Ret_>
struct ResultHelper {
  static void Helper(boost::function<Ret_()> func) {
    Deferred<Ret_> def(GetCurrentCoroutine().d_ptr->d_ptr);
    def.resolve(func());
  }

  template<typename func_type>
  static Coroutine bind(const func_type &func) {
    return Coroutine(boost::bind(&ResultHelper::Helper, func));
  }
};

template<>
struct ResultHelper<void> {
  static void Helper(const boost::function<void()> &func) {
    func();
    GetCurrentCoroutine().d_ptr->resolve();
  }

  template<typename func_type>
  static Coroutine bind(const func_type &func) {
    return Coroutine(func);
  }
};


template<typename func_type>
CoroutineT<typename func_type::result_type> Async__(const func_type &func) {
  typedef typename func_type::result_type Ret_;
  Coroutine co = ResultHelper<Ret_>::bind(func);
  co.run();
  return CoroutineT<Ret_>(co.d_ptr);
}

#endif //__COROUTINE_COROUTINE_H__
