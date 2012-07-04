#include "deferred.h"

Deferred<void>::Deferred()
	: DeferredBase()
{
}

//explicit
Deferred<void>::Deferred(DeferredContextInterface *ctx)
	: DeferredBase(ctx)
{
}

Deferred<void>::Deferred(const Deferred &rhs)
	: DeferredBase(rhs)
{
}

void Deferred<void>::resolve()
{
	if(!indeterminate(d_ptr->state())) {
		return;
	}
  d_ptr->resolve();
}

void Deferred<void>::reject()
{
	if(!indeterminate(d_ptr->state())) {
		return;
	}
  d_ptr->reject();
}
