#ifndef __COROUTINE_ASIO_H__
#define __COROUTINE_ASIO_H__

#include <boost/shared_array.hpp>

#if defined(WIN32)

#include "asio_win32.h"

#elif defined(__linux__)

#include "asio_linux.h"

#else
#error no asio support on your platform
#endif

#endif //__COROUTINE_ASIO_H__
