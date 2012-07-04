#ifndef __COROUTINE_SYNC_H__
#define __COROUTINE_SYNC_H__
#pragma once


#ifndef USE_QT_INFRASTRUCTURES

class DW_COROUTINE_EXPORT Mutex
{
	public:
		Mutex();
		~Mutex();
		void Lock();
		void Unlock();

	private:
		CRITICAL_SECTION crit_sec_;
};

class DW_COROUTINE_EXPORT MutexLock
{
	public:
		MutexLock(Mutex* m) : mu_(m) {mu_->Lock();}
		~MutexLock() {mu_->Unlock();}
	private:
		Mutex* mu_;
};


class DW_COROUTINE_EXPORT Semaphore
{
	public:
		explicit Semaphore(long initialCount = 0, long maxCount = std::numeric_limits<long>::max());
		~Semaphore();
		long release();
		bool acquire(/* int n = 1 */);

	private:
		HANDLE m_handle;
};

#else //USE_QT_INFRASTRUCTURES

# include <QMutex>
# include <QMutexLocker>
# include <QSemaphore>
typedef QMutex Mutex;
typedef QMutexLocker MutexLock;
typedef QSemaphore Semaphore;

#endif //USE_QT_INFRASTRUCTURES

#define MutexLock(x) STATIC_ASSERT(false && "mutex lock decl missing var name")

#endif //__COROUTINE_SYNC_H__
