#ifndef __COROUTINE_COROUTINE_P_H__
#define __COROUTINE_COROUTINE_P_H__

#pragma once

#include <windows.h>
#include <exception>
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "global.h"
#include "deferred.h"


#ifndef CACHED_FIBERS_LIMIT
#define CACHED_FIBERS_LIMIT 16
#endif //CACHED_FIBERS_LIMIT

class CancelAsyncException : public std::exception
{
};

typedef boost::function<void()> coroutine_proc_type;
class CoroutineData;
typedef comptr<CoroutineData> CoroutineDataPtr;

//internal coroutine data
class CoroutineData
{
	friend class Coroutine;
	friend class CoroutineManager;

	public:
		CoroutineData(const coroutine_proc_type &func);
		~CoroutineData();
		//we have thread affinity, and won't run in other thread
		//so, interlockXXX is not neccessary
		void AddRef() { ++m_refcount;}
		void Release() { if(--m_refcount == 0) delete this;}

		bool isActive() { return NULL != m_next; }
		bool isCancelled() { return m_cancelled; }
		Promise<> promise() { return m_deferred.promise(); }

		static void cancel_me(void* ctx);

	private:
		//don't call me directly
		void run();

		int											m_refcount;
		DWORD										m_threadId;
		coroutine_proc_type			m_func;
		LPVOID									m_fiber;
		Deferred<>							m_deferred;
		bool										m_cancelled;

		CoroutineDataPtr				m_next;
};


class CoroutineManager
{
	public:
		CoroutineManager();

		CoroutineDataPtr top() {return m_stack;}
		void schedule(CoroutineData *co);
		void yield(CoroutineData *co);

		static CoroutineManager& getInstance();
		static VOID CALLBACK FiberProc(PVOID unused);

	protected:
		void pop();
		void push(CoroutineDataPtr c);
		LPVOID allocateFiber();
		void freeFiber(LPVOID f);

	private:
		CoroutineDataPtr				m_main;
		CoroutineDataPtr				m_stack;
		LPVOID									m_freeFiberList[CACHED_FIBERS_LIMIT];
		int											m_recursiveDepth;
		int											m_freeFiberCount;
};

#endif //__COROUTINE_COROUTINE_P_H__
