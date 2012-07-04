#ifndef __COROUTINE_ASYNC_JOBPOOL_P_H__
#define __COROUTINE_ASYNC_JOBPOOL_P_H__
#pragma once

#include "promise/promise.h"


class ComputationalJob;
class ITask;

class JobPool
{
	public:
		Promise<ITask*> addJob(ITask* job);
		bool removeJob(ComputationalJob *job);

		static JobPool& getInstance();
#if defined(WIN32)
		static unsigned int CALLBACK WorkerProc(void *ctx);
#elif defined(POSIX)
		static void* WorkerProc(void *ctx);
#endif

	private:
		JobPool();
		void init();

		void push(ComputationalJob *job);
		ComputationalJob *pop();

	private:
		ComputationalJob		*m_queue;
		Mutex								m_mutex;
		Semaphore						m_semaphore;

		unsigned						m_workerCount;
		ThreadId						*m_workers;
};

class ComputationalJob
	: DeferredContext<ITask*>
{
	public:
		ComputationalJob(ITask* t);
		~ComputationalJob();
		void run();

	public:
		//from DeferredContext
		virtual void cancel_me();

#if defined(WIN32)
		static void CALLBACK CompleteRoutine(ULONG_PTR  ctx);
#elif defined(POSIX)
		void CompleteRoutine();
#endif

	private:
		ITask								*m_task;
		ThreadId						m_invoker;
		bool								m_cancelled;

		ComputationalJob		**m_prev;
		ComputationalJob		*m_next;

		friend class JobPool;
};

#endif //__COROUTINE_ASYNC_JOBPOOL_P_H__
