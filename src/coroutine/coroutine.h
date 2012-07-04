#ifndef __COROUTINE_COROUTINE_H__
#define __COROUTINE_COROUTINE_H__

#pragma once

#include <boost/bind.hpp>
#include "promise/promise.h"
typedef boost::function<void()> coroutine_proc_type;


class CoroutineData;
typedef boost::shared_ptr<CoroutineData> CoroutineDataPtr;

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

	private:
    ThreadId threadId();

		CoroutineDataPtr d_ptr;
};

DW_COROUTINE_EXPORT Coroutine GetCurrentCoroutine();

DW_COROUTINE_EXPORT Coroutine Async__(const coroutine_proc_type &func);

#define async(fna) Async__(boost::bind fna).run()

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

#endif //__COROUTINE_COROUTINE_H__
