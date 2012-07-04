#include "coroutine.h"
#include "coroutine_p.h"
#include "misc/threadlocal.h"


static ThreadLocal<CoroutineManager*> coroutineManagerInstance;

CoroutineData::CoroutineData(const coroutine_proc_type &func)
	: m_threadId(GetCurrentThreadId()), m_func(func),
		m_fiber(NULL), m_cancelled(false), m_next()
{
}

CoroutineData::~CoroutineData()
{
}

//virtual
void CoroutineData::cancel_me()
{
	//cancelling an activated coroutine makes no sence, just return
	//cancelling a coroutine which is never run before make no sence either
	ASSERT(!this->isActive() && this->m_fiber != NULL);
  if(!this->isActive() && this->m_fiber != NULL) {
    this->m_cancelled = true;
    CoroutineManager::getInstance().schedule(shared_from_this());
  }
}

void CoroutineData::run()
{
	THREAD_AFFINITY_CHECK();
	m_func();
}

//static
CoroutineManager& CoroutineManager::getInstance()
{
	CoroutineManager *pInstance = coroutineManagerInstance;
	if(!pInstance) {
		pInstance = new CoroutineManager();
		coroutineManagerInstance.set(pInstance);
	}
	return *pInstance;
}

void CoroutineManager::pop()
{
	ASSERT(m_main != m_stack);
	CoroutineDataPtr cur = m_stack;
	m_stack = cur->m_next;
	cur->m_next.reset();
	--m_recursiveDepth;
}

void CoroutineManager::push(CoroutineDataPtr c)
{
	//we are asymmetric coroutine, so, run again before yielding is not allow
	ASSERT(NULL == c->m_next);
	c->m_next = m_stack;
	m_stack = c;
	++m_recursiveDepth;
}
