#include "global.h"
#include "async_jobpool.h"
#include "async_jobpool_p.h"
#include "promise/deferred_context.h"


ITask::~ITask()
{
}

//static
#if defined(WIN32)
unsigned int CALLBACK JobPool::WorkerProc(void* /* = unused */)
#elif defined(POSIX)
void* JobPool::WorkerProc(void* /* = unused */)
#endif
{
	JobPool &jp = JobPool::getInstance();
	while(true) {
		jp.m_semaphore.acquire();
		ComputationalJob *job = jp.pop();
		job->run();
	}
  return 0;
}

JobPool::JobPool()
	: m_queue(NULL), m_mutex(), m_semaphore()
{
}


//static
JobPool& JobPool::getInstance()
{
	static JobPool *instance = NULL;
	if(!instance) {
		instance = new JobPool();
		instance->init();
	}
	return *instance;
}

Promise<ITask*> JobPool::addJob(ITask* job)
{
	ComputationalJob *j = new ComputationalJob(job);
	this->push(j);
	m_semaphore.release();
	return j->promise();
}

void JobPool::push(ComputationalJob *job)
{
	MutexLock l(&m_mutex);

	job->m_prev = &m_queue;
	job->m_next = m_queue;
	m_queue = job;
}

ComputationalJob* JobPool::pop()
{
	MutexLock l(&m_mutex);

	ComputationalJob *p = m_queue;

	if(p) {
		ComputationalJob *pp = p->m_next;
		if(pp) {
			pp->m_prev = &m_queue;
		}
		p->m_next = NULL;
		p->m_prev = NULL;
		m_queue = pp;
	}

	return p;
}

bool JobPool::removeJob(ComputationalJob *job)
{
	MutexLock l(&m_mutex);

	//not on the queue any more
	if(job->m_next == NULL && job->m_prev == NULL) {
		return false;
	}

	if(job->m_next) {
		job->m_next->m_prev = job->m_prev;
	}
	*job->m_prev = job->m_next;
	job->m_next = NULL;
	job->m_prev = NULL;

	return true;
}

ComputationalJob::ComputationalJob(ITask* t)
	: m_task(t), m_invoker(GetCurrentThreadId()),
		m_cancelled(false), m_prev(NULL), m_next(NULL)
{
}

ComputationalJob::~ComputationalJob()
{
  if(m_task) {
    delete m_task;
  }
}

//virtual
void ComputationalJob::cancel_me()
{
	this->m_cancelled = true;
	if(JobPool::getInstance().removeJob(this)) {
		d_ptr->setContext(NULL);
		delete this;
	}
}

Promise<ITask*> calculateAsync(ITask* task)
{
	return JobPool::getInstance().addJob(task);
}
