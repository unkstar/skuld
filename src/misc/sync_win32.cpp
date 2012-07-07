#include "global.h"

Mutex::Mutex()
{
	InitializeCriticalSection(&mutex_);
}

Mutex::~Mutex()
{
	DeleteCriticalSection(&mutex_);
}

void Mutex::Lock()
{
	::EnterCriticalSection(&mutex_);
}

void Mutex::Unlock()
{
	::LeaveCriticalSection(&mutex_);
}

Semaphore::Semaphore(long initialCount /* = 0 */)
	: sem_(NULL)
{
 sem_ = ::CreateSemaphore(NULL, initialCount, std::numeric_limits<long>::max(), NULL);
}

Semaphore::~Semaphore()
{
	::CloseHandle(sem_);
}

void Semaphore::release()
{
	::ReleaseSemaphore(sem_, 1, NULL);
}

bool Semaphore::acquire()
{
	DWORD ret = ::WaitForSingleObject(sem_, INFINITE);
	if(WAIT_OBJECT_0 == ret || WAIT_ABANDONED == ret) {
		return true;
	}
	return false;
}
