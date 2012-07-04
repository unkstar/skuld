// vim mode line, please do NOT remove
// vim: set filetype=cpp expandtab tabstop=2 autoindent smartindent:

template<typename Ty_>
DeferredContext<Ty_>::DeferredContext()
  : DeferredBase(this), DeferredContextInterface()
{
}

//virtual
template<typename Ty_>
DeferredContext<Ty_>::~DeferredContext() /*  = 0 */
{
  this->setContext(NULL);
}

//virtual
template<typename Ty_>
tribool DeferredContext<Ty_>::reset_me()
{
  return this->state();
}

template<typename Ty_>
Promise<Ty_> DeferredContext<Ty_>::promise()
{
  return Promise<Ty_>(d_ptr);
}

template<typename Ty_>
void DeferredContext<Ty_>::resolve(const Ty_ &res)
{
	if(!indeterminate(d_ptr->state())) {
		return;
	}
  d_ptr->setResult(res);
  d_ptr->resolve();
}

template<typename Ty_>
void DeferredContext<Ty_>::reject()
{
	if(!indeterminate(d_ptr->state())) {
		return;
	}
  d_ptr->reject();
}
