#include <pthread.h>

#include "global.h"
#include "async_jobpool.h"
#include "async_jobpool_p.h"
#include "promise/deferred_context.h"
#include "misc/sysinfo.h"




void JobPool::init()
{
	m_workerCount = getProcessorCount()  + 1;
	m_workers = new ThreadId[m_workerCount];

	for(unsigned i = 0; i < m_workerCount; ++i) {
		pthread_create(m_workers + i, NULL, &JobPool::WorkerProc, NULL);
	}
}

void ComputationalJob::run()
{
  if(!m_cancelled) {
    m_task->run();
  }
  sigval sv;
  sv.sival_ptr = this;
  if(0 != pthread_sigqueue(m_invoker, JOB_IS_DONE_SIGNAL, sv)) {
    delete this;
  }
}

void ComputationalJob::CompleteRoutine()
{
	if(!m_cancelled) {
		resolve(m_task);
	}
}
