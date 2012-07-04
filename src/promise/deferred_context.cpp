#include "deferred_context.h"
#include "misc/leak_detect.h"

DeferredContextInterface::DeferredContextInterface()
{
	ADD_LEAK_COUNTER_DETAIL(this);
}

//virtual
DeferredContextInterface::~DeferredContextInterface()
{
	REMOVE_LEAK_COUNTER_DETAIL(this);
}

//virtual
void DeferredContextInterface::cancel_me()
{
	ASSERT(false && "Unimplemented!");
}

DeferredContext<void>::DeferredContext()
  : DeferredBase(this), DeferredContextInterface()
{
}

//virtual
DeferredContext<void>::~DeferredContext() /*  = 0 */
{
  this->setContext(NULL);
}

//virtual
tribool DeferredContext<void>::reset_me()
{
  return this->state();
}

void DeferredContext<void>::resolve()
{
	if(!indeterminate(d_ptr->state())) {
		return;
	}
  d_ptr->resolve();
}

void DeferredContext<void>::reject()
{
	if(!indeterminate(d_ptr->state())) {
		return;
	}
  d_ptr->reject();
}
