#include "misc/sync.h"
#include <windows.h>

Mutex::Mutex()
{
	InitializeCriticalSection(&crit_sec_);
}

Mutex::~Mutex()
{
	DeleteCriticalSection(&crit_sec_);
}

void Mutex::Lock()
{
	::EnterCriticalSection(&crit_sec_);
}

void Mutex::Unlock()
{
	::LeaveCriticalSection(&crit_sec_);
}

Semaphore::Semaphore(long initialCount /* = 0 */, long maxCount /* = std::numeric_limits<long>::max()*/)
	: m_handle(NULL)
{
 m_handle = ::CreateSemaphore(NULL, initialCount, maxCount, NULL);
}

Semaphore::~Semaphore()
{
	::CloseHandle(m_handle);
}

long Semaphore::release()
{
	LONG prev = -1;
	::ReleaseSemaphore(m_handle, 1, &prev);
	return prev;
}

bool Semaphore::acquire(/* int n = 1 */)
{
	DWORD ret = ::WaitForSingleObject(m_handle, INFINITE);
	if(WAIT_OBJECT_0 == ret || WAIT_ABANDONED == ret) {
		return true;
	}
	return false;
}
