#include "coroutine.h"
#include "coroutine_p.h"
#include "promise.h"


#define THREAD_AFFINITY_CHECK() ASSERT(d_ptr->m_threadId == GetCurrentThreadId())


Coroutine::Coroutine(const coroutine_proc_type &func)
	: d_ptr(new CoroutineData(func))
{
	d_ptr->AddRef();
}

Coroutine::Coroutine(const Coroutine& rhs)
	: d_ptr(rhs.d_ptr)
{
	THREAD_AFFINITY_CHECK();
	d_ptr->AddRef();
}

Coroutine::Coroutine(CoroutineData *d)
	: d_ptr(d)
{
	THREAD_AFFINITY_CHECK();
	d_ptr->AddRef();
}

Coroutine::~Coroutine()
{
	THREAD_AFFINITY_CHECK();
	if(d_ptr) {
		d_ptr->Release();
	}
}

Coroutine& Coroutine::run()
{
	THREAD_AFFINITY_CHECK();
	ASSERT(!d_ptr->isActive());

	CoroutineManager::getInstance().schedule(d_ptr);
	return *this;
}

Coroutine& Coroutine::yield()
{
	THREAD_AFFINITY_CHECK();
	ASSERT(d_ptr->isActive());

	CoroutineManager::getInstance().yield(d_ptr);
	return *this;
}

bool Coroutine::isCancelled()
{
	return d_ptr->isCancelled();
}

Promise<> Coroutine::promise()
{
	return d_ptr->promise();
}

Coroutine GetCurrentCoroutine()
{
	return Coroutine(CoroutineManager::getInstance().top().get());
}

Coroutine Async__(const coroutine_proc_type &func)
{
	return Coroutine(func);
}

