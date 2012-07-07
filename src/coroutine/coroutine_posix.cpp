#include "coroutine.h"
#include "coroutine_p.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>

# define AllocateFiber(s) (fiber_t*)mmap(0, sizeof(fiber_t) + s, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
# define FreeFiber(p, s) munmap(p, s + sizeof(fiber_t))

static void* dummy()
{
	return 0;
}

CoroutineManager::CoroutineManager()
	:m_recursiveDepth(0), m_freeFiberCount(0), m_stackSize(0)
{
	memset(m_freeFiberList, 0, CACHED_COROUTINE_LIMIT * sizeof(FiberType));

  //TODO:error handling, pthread_attr_xxx may fail because of ENOMEM
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_getstacksize(&attr, &m_stackSize);
  pthread_attr_destroy(&attr);

	this->m_main.reset(new CoroutineData(dummy));
  //TODO:add some code to release main fiber
	this->m_main->m_fiber = AllocateFiber(0); //main context use thread's own stack
	this->push(m_main);
}

//static
void CoroutineManager::FiberProc()
{
  FiberType ret = NULL;
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
  setcontext(&ret->context);
}


FiberType CoroutineManager::allocateFiber()
{
	FiberType fiber = NULL;
	if(0 != m_freeFiberCount) {
		fiber = m_freeFiberList[--m_freeFiberCount];
		m_freeFiberList[m_freeFiberCount] = NULL;
	} else {
		fiber = AllocateFiber(m_stackSize);
    if(MAP_FAILED == fiber) {
      printf("%s:(%d)", strerror(errno), (int)errno);
    }
	}
  getcontext(&fiber->context);
  fiber->context.uc_stack.ss_sp = fiber->stack;
  fiber->context.uc_stack.ss_size = m_stackSize;
  makecontext(&fiber->context, &CoroutineManager::FiberProc, 0);
	return fiber;
}

void CoroutineManager::freeFiber(FiberType fiber)
{
	if(m_freeFiberCount < CACHED_COROUTINE_LIMIT) {
		//post fix incr, a little tricky?
		m_freeFiberList[m_freeFiberCount++] = fiber;
	} else {
		//we delete the last cached one, because if we delete current one, currently activated stack will corrupt
		FreeFiber(m_freeFiberList[CACHED_COROUTINE_LIMIT - 1], m_stackSize);
		m_freeFiberList[CACHED_COROUTINE_LIMIT - 1] = fiber;
	}
}

void CoroutineManager::schedule(CoroutineDataPtr co)
{
	if(NULL == co->m_fiber) {
		co->m_fiber = allocateFiber();
	}
  CoroutineDataPtr prev = this->top();
	this->push(co);
  //XXX:return?
  swapcontext(&prev->m_fiber->context, &co->m_fiber->context);
}

void CoroutineManager::yield(CoroutineDataPtr co)
{
	ASSERT(m_stack == co);
  CoroutineDataPtr cur = this->top();
	this->pop();
	swapcontext(&cur->m_fiber->context, &m_stack->m_fiber->context);
}
