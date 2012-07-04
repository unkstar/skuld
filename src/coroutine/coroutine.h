#ifndef __COROUTINE_COROUTINE_H__
#define __COROUTINE_COROUTINE_H__

#pragma once

#include "boost/bind.hpp"
#include "coroutine_p.h"
#include "promise.h"

#define async(fna) Async__(boost::bind fna).run()

class CoroutineData;


/*****************************************************************************
 * This coroutine is build on top of Windows fiber, with two restriction:
 * 1) Add thread affinity to fiber, we don't allow fiber run on
 *		other thread rather than the creation one.
 * 2) Make it asymmetric, Windows fibers are symmetric,
 *		but we prefer LUA like asymmetric implementation
 *****************************************************************************/
class Coroutine
{
	public:
		explicit Coroutine(const coroutine_proc_type &func);
		explicit Coroutine(CoroutineData *d);
		//copy constructable
		Coroutine(const Coroutine &rhs);
		~Coroutine();

		//chainable
		Coroutine& run();
		Coroutine& yield();
		bool isCancelled();

		Promise<> promise();

	private:
		CoroutineData *d_ptr;
};

Coroutine GetCurrentCoroutine();

Coroutine Async__(const coroutine_proc_type &func);

//also, chainable
template<typename Ty_>
Promise<Ty_>& await(Promise<Ty_>& pro)
{
	Coroutine cur = GetCurrentCoroutine();
	size_t h = pro.always(boost::bind(&Coroutine::run, cur));
	cur.yield();
	pro.removeAlways(h);
	if(cur.isCancelled()) {
		throw CancelAsyncException();
	}
	return pro;
}

#endif //__COROUTINE_COROUTINE_H__
