#include "promise.h"

//all combinators in this file are cloned of
//"The Task-based Asynchronous Pattern"
//Stephen Toub, Microsoft
//February 2012
//http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=19957

/*
 * retry combinator
 * usage example:
 *	Promise<std::string> downloadStringAsync(const std::string &url)
 *	{
 *	//blah blah
 *	}
 *
 *	std::string pageContent = retryOnFault(boost::bind(downloadStringAsync, url), 3, boost::bind(setTimeout, 900));
 */

template<typename result_type>
result_type retryOnFault(boost::function<result_type()> func, int count)
{
	for(int i = 0; i < count; ++i) {
		try {
			return func();
		} catch(...) {
			if(i == count - 1) throw;
		}
	}
}

template<typename result_type>
result_type retryOnFault(boost::function<Promise<result_type>()> func, int count, boost::function<Promise<void>()> whenToRetry)
{
	for(int i = 0; i < count; ++i) {
		try {
			Promise<result_type> r = await(func());
			if(r.state()) {
				return r.result();
			}
		} catch(...) {
			if(i == count - 1) throw;
		}
		await(whenToRetry());
	}
}

template<typename result_type, int count_>
result_type needOnlyOne(boost::function<Promise<result_type>()> sources[count_])
{
	AutoCancelPromise<result_type> promises[count_];
	Promise<> whenAny(false);

	promises[0] = sources[0]();
	whenAny = promises[0];
	for(int i = 1; i < count_; ++i) {
		promises[i] = sources[i]();
		whenAny ||= promises[i];
	}

	await(whenAny);

	if(whenAny.state()) {
		for(int i = 0; i < count_; ++i) {
			if(promises[i].state()) {
				return promises[i].result();
			}
		}
	}

	throw std::exception("all request failed");
}

//C++ does not support in-place array
boost::function<Promise<double>()> servers[] = {
	boost::bind(getCurrentPriceFromServer1, "msft"),
	boost::bind(getCurrentPriceFromServer2, "msft"),
	boost::bind(getCurrentPriceFromServer3, "msft"),
}

double price = needOnlyOne(servers);

//..crazy ideas!
template<typename arg_type>
void func(arg_type arg1, ...)
{
	var_list vararg;
}

#define DefineAsyncFunction(r, n, a) \
	AutoBindPromise<r> n a {\
		\\blah blah \
	}
