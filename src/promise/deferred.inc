// vim mode line, please do NOT remove
// vim: set filetype=cpp expandtab tabstop=2 autoindent smartindent:
#include "deferred_p.h"

template<typename Ty_>
Deferred<Ty_>::Deferred()
	: DeferredBase()
{
}

//explicit
template<typename Ty_>
Deferred<Ty_>::Deferred(DeferredDataPtr d)
  : DeferredBase(d)
{
}

//explicit
template<typename Ty_>
Deferred<Ty_>::Deferred(DeferredContextInterface *ctx)
	: DeferredBase(ctx)
{
}

template<typename Ty_>
Deferred<Ty_>::Deferred(const Deferred &rhs)
	: DeferredBase(rhs)
{
}

template<typename Ty_>
void Deferred<Ty_>::resolve(const Ty_ &res)
{
	if(!indeterminate(d_ptr->state())) {
		return;
	}
  d_ptr->setResult(res);
  d_ptr->resolve();
}

template<typename Ty_>
void Deferred<Ty_>::reject()
{
	if(!indeterminate(d_ptr->state())) {
		return;
	}
  d_ptr->reject();
}
