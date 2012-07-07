#ifndef __COROUTINE_COROUTINE_P_H__
#define __COROUTINE_COROUTINE_P_H__

#pragma once

#include "../global.h"

#include <exception>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "promise/promise.h"
#include "promise/deferred.h"
#include "promise/deferred_context.h"
#include "coroutine/coroutine.h"


#ifndef CACHED_COROUTINE_LIMIT
#define CACHED_COROUTINE_LIMIT 16
#endif //CACHED_COROUTINE_LIMIT

#ifndef CALLBACK
#define CALLBACK
#endif

#define THREAD_AFFINITY_CHECK() ASSERT(this->threadId() == GetCurrentThreadId())


//internal coroutine data
class DW_COROUTINE_EXPORT CoroutineData
	: public DeferredContext<void>,
    public boost::enable_shared_from_this<CoroutineData>
{
	friend class Coroutine;
	friend class CoroutineManager;

  template<typename Ty_>
    friend class CoroutineT;

  template<typename Ret_>
    friend struct ResultHelper;

	public:
		CoroutineData(const coroutine_proc_type &func);
		~CoroutineData();

		bool isActive() { return NULL != m_next; }
		bool isCancelled() { return m_cancelled; }

		virtual void cancel_me();

	private:
		//don't call me directly
		void run();

    ThreadId threadId() { return m_threadId; }

		ThreadId								m_threadId;
		coroutine_proc_type			m_func;
		FiberType								m_fiber;
		bool										m_cancelled;

		CoroutineDataPtr				m_next;
};


class DW_COROUTINE_EXPORT CoroutineManager
{
	public:
		CoroutineManager();

		CoroutineDataPtr top() {return m_stack;}
		void schedule(CoroutineDataPtr co);
		void yield(CoroutineDataPtr co);

		static CoroutineManager& getInstance();
#ifdef WIN32
		static void CALLBACK FiberProc(void *unused);
#endif
#ifdef POSIX
		static void FiberProc();
#endif

	protected:
		void pop();
		void push(CoroutineDataPtr c);
		FiberType allocateFiber();
		void freeFiber(FiberType f);

	private:
		CoroutineDataPtr				m_main;
		CoroutineDataPtr				m_stack;
		FiberType								m_freeFiberList[CACHED_COROUTINE_LIMIT];
		int											m_recursiveDepth;
		int											m_freeFiberCount;
#ifdef POSIX
    size_t                  m_stackSize;
#endif
};

#endif //__COROUTINE_COROUTINE_P_H__
