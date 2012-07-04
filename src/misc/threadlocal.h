#ifndef __COROUTINE_THREAD_LOCAL_H__
#define __COROUTINE_THREAD_LOCAL_H__
#pragma once

#if defined(WIN32)
# include <windows.h>
#elif defined(LINUX) || defined(__APPLE__)
# include <pthread.h>
#endif


#if defined(WIN32)
//TODO:write something to dllmain to do threadlocal clean up job
template<typename Ty_>
struct ThreadLocal {
	ThreadLocal() {
		slot = ::TlsAlloc();
	}

	~ThreadLocal() {
		::TlsFree(slot);
	}

	operator Ty_() {
		return (Ty_)::TlsGetValue(slot);
	}

	void set(Ty_ v) {
		STATIC_ASSERT(sizeof(Ty_) == sizeof(LPVOID));
		::TlsSetValue(slot, (LPVOID)v);
	}

	DWORD slot;
};

#elif defined(__linux__) || defined(__APPLE__)
template<typename Ty_>
struct ThreadLocal {
	ThreadLocal() {
    pthread_key_create(&slot, FinalizeKey);
	}

	~ThreadLocal() {
    pthread_key_delete(slot);
  }

	static void FinalizeKey(void *entry) {
    delete (Ty_)entry;
	}

	operator Ty_() {
		return (Ty_)::pthread_getspecific(slot);
	}

	void set(Ty_ v) {
		STATIC_ASSERT(sizeof(Ty_) == sizeof(void*));
		::pthread_setspecific(slot, (void*)v);
	}

  pthread_key_t slot;
};

#endif

#endif //__COROUTINE_THREAD_LOCAL_H__
