#include "coroutine.h"
#include "coroutine_p.h"

static void* dummy()
{
	return 0;
}

CoroutineManager::CoroutineManager()
	:m_recursiveDepth(0), m_freeFiberCount(0)
{
	memset(m_freeFiberList, 0, CACHED_COROUTINE_LIMIT * sizeof(FiberType));

	this->m_main = new CoroutineData(dummy);
	//TODO:is floating point state switching really needed?
	this->m_main->m_fiber = ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
	this->push(m_main);
}

//static
VOID CALLBACK CoroutineManager::FiberProc(PVOID /* unused */)
{
	while(true) {
		FiberType ret = NULL;

		//do NOT use ANY CoroutineDataPtr or other non-POD type local variable out of do{...} while(0) scope
		//if this fiber is deleted after it goes back to free list,
		//stack variable will NOT destruct, refcount will not be release neither
		do {
			CoroutineManager& man = CoroutineManager::getInstance();
			CoroutineDataPtr cur = man.top();
			try {
				cur->run();
			} catch(CancelAsyncException&) {
				//hmmm..shall we do something?
			}
			if(!cur->isCancelled()) {
				cur->resolve();
			}

			//asymmetric principle check
			ASSERT(cur == man.top());
			man.pop();
			FiberType fiber = cur->m_fiber;
			cur->m_fiber = NULL;
			man.freeFiber(fiber);
			ret = man.top()->m_fiber;
		} while(0);

		//halt current fiber, it may never go back again
		SwitchToFiber(ret);
	}
}


FiberType CoroutineManager::allocateFiber()
{
	FiberType fiber = NULL;
	if(0 != m_freeFiberCount) {
		fiber = m_freeFiberList[--m_freeFiberCount];
		m_freeFiberList[m_freeFiberCount] = NULL;
	} else {
		fiber = CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, FiberProc, NULL);
	}
	return fiber;
}

void CoroutineManager::freeFiber(FiberType fiber)
{
	if(m_freeFiberCount < CACHED_COROUTINE_LIMIT) {
		//post fix incr, a little tricky?
		m_freeFiberList[m_freeFiberCount++] = fiber;
	} else {
		//we delete the last cached one, because if we delete current one, thread will terminate
		DeleteFiber(m_freeFiberList[CACHED_COROUTINE_LIMIT - 1]);
		m_freeFiberList[CACHED_COROUTINE_LIMIT - 1] = fiber;
	}
}

void CoroutineManager::schedule(CoroutineDataPtr co)
{
	if(NULL == co->m_fiber) {
		co->m_fiber = allocateFiber();
	}
	this->push(co);
	SwitchToFiber(co->m_fiber);
}

void CoroutineManager::yield(CoroutineDataPtr co)
{
	ASSERT(m_stack == co);
	this->pop();
	SwitchToFiber(m_stack->m_fiber);
}
