// vim mode line, please do NOT remove
// vim: set filetype=cpp expandtab tabstop=2 autoindent smartindent:
template<typename Ty_>
Promise<Ty_>::Promise()
  : d_ptr(&DeferredData::shared_indeterminate_deferred)
{
  d_ptr->AddRef();
}

template<typename Ty_>
Promise<Ty_>::Promise(DeferredData *d)
	: d_ptr(d)
{
  if(d_ptr) {
    d_ptr->AddRef();
  }
}

template<typename Ty_>
Promise<Ty_>::Promise(const Promise &rhs)
	: d_ptr(rhs.d_ptr)
{
  if(d_ptr) {
    d_ptr->AddRef();
  }
}

template<typename Ty_>
Promise<Ty_>::~Promise()
{
	d_ptr->Release();
}

template<typename Ty_>
size_t Promise<Ty_>::done(const promise_callback_type &func)
{
	return d_ptr->done(func);
}

template<typename Ty_>
size_t Promise<Ty_>::fail(const promise_callback_type &func)
{
	return d_ptr->fail(func);
}

template<typename Ty_>
size_t Promise<Ty_>::always(const promise_always_type &func)
{
	return d_ptr->always(func);
}

template<typename Ty_>
void Promise<Ty_>::removeDone(size_t handle)
{
	return d_ptr->removeDone(handle);
}

template<typename Ty_>
void Promise<Ty_>::removeFail(size_t handle)
{
	return d_ptr->removeFail(handle);
}

template<typename Ty_>
void Promise<Ty_>::removeAlways(size_t handle)
{
	return d_ptr->removeAlways(handle);
}


template<typename Ty_>
bool Promise<Ty_>::isRejected()
{
	 return d_ptr->isRejected();
}

template<typename Ty_>
bool Promise<Ty_>::isResolved()
{
	 return d_ptr->isResolved();
}

template<typename Ty_>
tribool Promise<Ty_>::state()
{
	 return d_ptr->state();
}

template<typename Ty_>
Ty_ Promise<Ty_>::result()
{
	return d_ptr->result().value<Ty_>();
}
template<typename Ty_>
void Promise<Ty_>::cancel()
{
	 return d_ptr->cancel();
}
