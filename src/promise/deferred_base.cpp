#include "deferred_p.h"
#include "deferred_base.h"
#include "deferred_context.h"

DeferredBase::DeferredBase()
	: d_ptr(new DeferredData())
{
}


//explicit
DeferredBase::DeferredBase(DeferredDataPtr d)
	:d_ptr(d)
{
}

//explicit
DeferredBase::DeferredBase(DeferredContextInterface *ctx)
	: d_ptr(new DeferredData(ctx))
{
}

DeferredBase::DeferredBase(const DeferredBase &rhs)
	: d_ptr(rhs.d_ptr)
{
}

DeferredBase::~DeferredBase()
{
}


void DeferredBase::assign(const DeferredBase &rhs)
{
  d_ptr = rhs.d_ptr;
}

size_t DeferredBase::done(const promise_callback_type &func)
{
	return d_ptr->done(func);
}

size_t DeferredBase::fail(const promise_callback_type &func)
{
	return d_ptr->fail(func);
}

size_t DeferredBase::always(const promise_always_type &func)
{
	return d_ptr->always(func);
}

void DeferredBase::removeDone(size_t handle)
{
	return d_ptr->removeDone(handle);
}

void DeferredBase::removeFail(size_t handle)
{
	return d_ptr->removeFail(handle);
}

void DeferredBase::removeAlways(size_t handle)
{
	return d_ptr->removeAlways(handle);
}


bool DeferredBase::isRejected()
{
	 return d_ptr->isRejected();
}

bool DeferredBase::isResolved()
{
	 return d_ptr->isResolved();
}
bool DeferredBase::isActivating()
{
	return d_ptr->isActivating();
}

tribool DeferredBase::state() const
{
	 return d_ptr->state();
}

void DeferredBase::cancel()
{
	 return d_ptr->cancel();
}

void DeferredBase::resetState(tribool s)
{
  d_ptr->resetState(s);
}

void DeferredBase::setContext(DeferredContextInterface *ctx)
{
  d_ptr->setContext(ctx);
}
