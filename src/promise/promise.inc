// vim mode line, please do NOT remove
// vim: set filetype=cpp expandtab tabstop=2 autoindent smartindent:


#include "deferred_context.h"

static DeferredDataPtr getSharedDeferred(tribool s)
{
  if(s) {
    return DeferredData::shared_true_deferred;
  } else if(!s) {
    return DeferredData::shared_false_deferred;
  } else {
    return DeferredData::shared_indeterminate_deferred;
  }
}

template<typename Ty_>
Promise<Ty_>::Promise()
  : DeferredBase(getSharedDeferred(indeterminate))
{
}

template<typename Ty_>
Promise<Ty_>::Promise(bool s)
  : DeferredBase(getSharedDeferred(tribool(s)))
{
}

template<typename Ty_>
Promise<Ty_>::Promise(DeferredDataPtr d)
	: DeferredBase(d)
{
}

template<typename Ty_>
Promise<Ty_>::Promise(const Deferred<Ty_> &d)
	: DeferredBase(d)
{
}

template<typename Ty_>
Promise<Ty_>::Promise(const Promise &rhs)
	: DeferredBase(rhs)
{
}

template<typename Ty_>
Ty_ Promise<Ty_>::result()
{
  if(!d_ptr->result().empty()) {
    return boost::any_cast<Ty_>(d_ptr->result());
  }
  return Ty_();
}

template<typename Ty_>
tribool Promise<Ty_>::reset()
{
  return d_ptr->reset();
}
