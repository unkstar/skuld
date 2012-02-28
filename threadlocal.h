#ifndef __COROUTINE_THREAD_LOCAL_H__
#define __COROUTINE_THREAD_LOCAL_H__
#pragma once

#include <windows.h>


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

#endif //__COROUTINE_THREAD_LOCAL_H__
