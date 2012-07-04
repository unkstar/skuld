// vim mode line, please do NOT remove
// vim: set filetype=cpp expandtab tabstop=2 autoindent smartindent:
template<typename Ty_>
Deferred<Ty_>::Deferred()
	: d_ptr(new DeferredData())
{
	d_ptr->AddRef();
}

template<typename Ty_>
Deferred<Ty_>::Deferred(tribool s)
  : d_ptr(NULL)
{
  if(s) {
    d_ptr = &DeferredData::shared_true_deferred;
  } else if(!s) {
    d_ptr = &DeferredData::shared_false_deferred;
  } else {
    d_ptr = &DeferredData::shared_indeterminate_deferred;
  }
  d_ptr->AddRef();
}

template<typename Ty_>
Deferred<Ty_>::Deferred(const Deferred &rhs)
	: d_ptr(rhs.d_ptr)
{
	d_ptr->AddRef();
}

template<typename Ty_>
Deferred<Ty_>::~Deferred()
{
	d_ptr->Release();
}

template<typename Ty_>
void Deferred<Ty_>::resolve()
{
	d_ptr->resolve();
}

template<typename Ty_>
void Deferred<Ty_>::reject()
{
	d_ptr->reject();
}

template<typename Ty_>
size_t Deferred<Ty_>::done(const promise_callback_type &func)
{
	return d_ptr->done(func);
}

template<typename Ty_>
size_t Deferred<Ty_>::fail(const promise_callback_type &func)
{
	return d_ptr->fail(func);
}

template<typename Ty_>
size_t Deferred<Ty_>::always(const promise_always_type &func)
{
	return d_ptr->always(func);
}

template<typename Ty_>
void Deferred<Ty_>::removeDone(size_t handle)
{
	return d_ptr->removeDone(handle);
}

template<typename Ty_>
void Deferred<Ty_>::removeFail(size_t handle)
{
	return d_ptr->removeFail(handle);
}

template<typename Ty_>
void Deferred<Ty_>::removeAlways(size_t handle)
{
	return d_ptr->removeAlways(handle);
}


template<typename Ty_>
bool Deferred<Ty_>::isRejected()
{
	 return d_ptr->isRejected();
}

template<typename Ty_>
bool Deferred<Ty_>::isResolved()
{
	 return d_ptr->isResolved();
}

template<typename Ty_>
tribool Deferred<Ty_>::state()
{
	 return d_ptr->state();
}

template<typename Ty_>
void Deferred<Ty_>::cancel()
{
	 return d_ptr->cancel();
}
