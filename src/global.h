#ifndef __COROUTINE_GLOBAL_H__
#define __COROUTINE_GLOBAL_H__
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif //WIN32_LEAN_AND_MEAN

#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif

#if defined(WIN32)
#define DECL_EXPORT __declspec( dllexport )
#define DECL_IMPORT __declspec( dllimport )
#else
#define DECL_EXPORT
#define DECL_IMPORT
#endif

#  ifdef BUILD_DW_COROUTINE
#    define DW_COROUTINE_EXPORT DECL_EXPORT
#  else
#    define DW_COROUTINE_EXPORT DECL_IMPORT
#  endif // BUILD_DW_COROUTINE


#include <limits>
#include <boost/logic/tribool.hpp>
#include <boost/shared_ptr.hpp>


#if defined(WIN32)
# include <windows.h>
  typedef unsigned int ThreadId;
  typedef LPVOID FiberType;
  typedef CRITICAL_SECTION SYS_MUTEX_T;
  typedef HANDLE  SYS_SEM_T;

#elif defined(__linux__) || defined(__APPLE__)

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

# include <pthread.h>
# include <semaphore.h>
# include <ucontext.h>
# include <sys/mman.h>
  typedef pthread_t ThreadId;
  struct fiber_t {
    ucontext_t  context;
    char        stack[1];
  };
  typedef fiber_t* FiberType;

  typedef pthread_mutex_t SYS_MUTEX_T;
  typedef sem_t SYS_SEM_T;

# ifndef POSIX
#   define POSIX
# endif


# define GetCurrentThreadId() pthread_self()

#define JOB_IS_DONE_SIGNAL (SIGRTMIN + 1)
#define IO_SIGNAL (SIGRTMIN + 2)

#endif


//from Qt.Network.q_static_assert
namespace {
    template<bool> struct StaticAssertFailed;
    template<> struct StaticAssertFailed<true> { enum { Value = 1 }; };
}

static inline void noop_with_arg(int) {}
#define STATIC_ASSERT(expr)   noop_with_arg(sizeof(StaticAssertFailed< expr >::Value))


using boost::logic::tribool;
using boost::logic::indeterminate;

#include "misc/sync.h"

#endif //__COROUTINE_GLOBAL_H__
