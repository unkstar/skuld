#include "global.h"
#include "async_jobpool.h"
#include "async_jobpool_p.h"
#include "promise/deferred_context.h"
#include "misc/sysinfo.h"
#include <process.h>


void JobPool::init()
{
	m_workerCount = getProcessorCount()  + 1;
	m_workers = new ThreadId[m_workerCount];

	for(unsigned i = 0; i < m_workerCount; ++i) {
		_beginthreadex(NULL, 0, &JobPool::WorkerProc, NULL, 0, m_workers + i);
	}
}

void ComputationalJob::run()
{
	if(!m_cancelled) {
		m_task->run();
	}
	HANDLE h = ::OpenThread(THREAD_SET_CONTEXT, false, m_invoker);
	if(h) {
		if(QueueUserAPC(&ComputationalJob::CompleteRoutine, h, (ULONG_PTR)this)) {
			return;
		}
		::CloseHandle(h);
	}
	delete this;
}

void CALLBACK ComputationalJob::CompleteRoutine(ULONG_PTR  ctx)
{
	ComputationalJob *j = (ComputationalJob*)ctx;
	if(!j->m_cancelled) {
		j->resolve(j->m_task);
	}
	delete j;
}
