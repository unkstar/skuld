#ifndef __COROUTINE_ASYNC_JOBPOOL_H__
#define __COROUTINE_ASYNC_JOBPOOL_H__
#pragma once


#include "promise/promise.h"
#include <boost/function.hpp>

//async computational job pool
//NEVER PUT ANY I/O JOB IN IT, USE ASIO instead

class DW_COROUTINE_EXPORT ITask
{
	public:
		virtual ~ITask() = 0;
		virtual void run() = 0;
};

DW_COROUTINE_EXPORT Promise<ITask*> calculateAsync(ITask* task);

template<typename Ty_>
class FunctionTask
	: public ITask
{
	typedef boost::function<Ty_()> function_type;
	public:
		void run()
		{
		}
	private:
		function_type m_func;
};

template<typename Ty_>
Promise<Ty_> calculateAsync(boost::function<Ty_()> f)
{
}

#endif //__COROUTINE_ASYNC_JOBPOOL_H__
