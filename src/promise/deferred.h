#ifndef __COROUTINE_DEFERRED_H__
#define __COROUTINE_DEFERRED_H__
#pragma once

#include <boost/function.hpp>

#include "../global.h"
#include "deferred_base.h"

class DeferredData;
class DeferredContextInterface;

template<typename Ty_ = void>
class Deferred
	: public DeferredBase
{
	public:
		Deferred();
		explicit Deferred(DeferredDataPtr);
		explicit Deferred(DeferredContextInterface *ctx);
		Deferred(const Deferred &rhs);

		void resolve(const Ty_ &res);
		void reject();

		Deferred& operator=(const Deferred &rhs)
		{
      this->assign(rhs);
      return *this;
		}

};

template<>
class DW_COROUTINE_EXPORT Deferred<void>
	: public DeferredBase
{
  public:
    Deferred();
    explicit Deferred(DeferredContextInterface *ctx);
    Deferred(const Deferred &rhs);

    void resolve();
    void reject();

    Deferred& operator=(const Deferred &rhs)
    {
      this->assign(rhs);
      return *this;
    }
};



#include "deferred.inc"

#endif //__COROUTINE_DEFERRED_H__
