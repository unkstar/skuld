#include "coroutine_p.h"
#include "threadlocal.h"
#include <windows.h>

#define THREAD_AFFINITY_CHECK() ASSERT(m_threadId == GetCurrentThreadId())

static ThreadLocal<CoroutineManager*> coroutineManagerInstance;

static void* dummy()
{
	return 0;
}

CoroutineData::CoroutineData(const coroutine_proc_type &func)
	: m_refcount(0), m_threadId(GetCurrentThreadId()), m_func(func),
		m_fiber(NULL), m_deferred(), m_cancelled(false), m_next(NULL)
{
	m_deferred.setContext(this, &CoroutineData::cancel_me);
}

CoroutineData::~CoroutineData()
{
}

//static
void CoroutineData::cancel_me(void* ctx)
{
	CoroutineData* pThis = (CoroutineData*)ctx;
	//cancelling an activated coroutine makes no sence, just return
	//cancelling a coroutine which is never run bufore make no sence either
	ASSERT(!pThis->isActive() && pThis->m_fiber != NULL);
	pThis->m_cancelled = true;
	CoroutineManager::getInstance().schedule(pThis);
}

void CoroutineData::run()
{
	THREAD_AFFINITY_CHECK();
	m_func();
}

CoroutineManager::CoroutineManager()
	:m_recursiveDepth(0), m_freeFiberCount(0)
{
	memset(m_freeFiberList, 0, CACHED_FIBERS_LIMIT * sizeof(LPVOID));

	this->m_main = new CoroutineData(dummy);
	//TODO:is float point state swtiching really needed?
	this->m_main->m_fiber = ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
	this->push(m_main);
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
	cur->m_next = NULL;
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

//static
VOID CALLBACK CoroutineManager::FiberProc(PVOID /* unused */) {
	while(true) {
		LPVOID ret = NULL;

		//do NOT use ANY CoroutineDataPtr or other non-POD type local variable out of do{...} while(0) scope
		//if this fiber is deleted after it goes back to free list,
		//stack variable will NOT destruct, refcount will not be release neither
		do {
			CoroutineManager& man = CoroutineManager::getInstance();
			CoroutineDataPtr cur = man.top();
			try {
				cur->run();
			} catch(CancelAsyncException& e) {
				//hmmm..shall we do something?
			}
			if(!cur->isCancelled()) {
				cur->m_deferred.resolve();
			}

			//asymmetric principle check
			ASSERT(cur == man.top());
			man.pop();
			LPVOID fiber = cur->m_fiber;
			cur->m_fiber = NULL;
			man.freeFiber(fiber);
			ret = man.m_stack->m_fiber;
		} while(0);

		//halt current fiber, it may never go back again
		SwitchToFiber(ret);
	}
}


LPVOID CoroutineManager::allocateFiber()
{
	LPVOID fiber = NULL;
	if(0 != m_freeFiberCount) {
		fiber = m_freeFiberList[--m_freeFiberCount];
	} else {
		fiber = CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, FiberProc, NULL);
	}
	return fiber;
}

void CoroutineManager::freeFiber(LPVOID fiber)
{
	if(m_freeFiberCount < CACHED_FIBERS_LIMIT) {
		//post fix incr, a little tricky?
		m_freeFiberList[m_freeFiberCount++] = fiber;
	} else {
		//we delete the last cached one, because if we delete current one, thread will terminate
		DeleteFiber(m_freeFiberList[CACHED_FIBERS_LIMIT - 1]);
		m_freeFiberList[CACHED_FIBERS_LIMIT - 1] = fiber;
	}
}

void CoroutineManager::schedule(CoroutineData *co)
{
	if(NULL == co->m_fiber) {
		co->m_fiber = allocateFiber();
	}
	this->push(co);
	SwitchToFiber(co->m_fiber);
}

void CoroutineManager::yield(CoroutineData *co)
{
	ASSERT(m_stack == co);
	this->pop();
	SwitchToFiber(m_stack->m_fiber);
}
