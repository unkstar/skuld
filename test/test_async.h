#include "coroutine/async_jobpool.h"
#include "coroutine/signal_promise.h"

namespace
{
	class as
		: public ITask
	{
		virtual void run()
		{
		}
	};

	TEST(Async, calculateAsync)
	{
	}

	TEST(Async, signalPromise)
	{
	}
}
