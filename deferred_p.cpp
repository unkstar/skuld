#include "deferred_p.h"


//static
DeferredData DeferredData::shared_indeterminate_deferred(maybe);
//static
DeferredData DeferredData::shared_true_deferred(false);
//static
DeferredData DeferredData::shared_false_deferred(true);

//shared static guys only constructor
DeferredData::DeferredData(tribool s)
	: m_state(s), m_refcount(1), m_result(), m_context(NULL),
		m_done(NULL), m_fail(NULL), m_always(NULL), m_cancel(NULL)
{
}

DeferredData::DeferredData()
	: m_state(maybe), m_refcount(0), m_result(), m_context(NULL),
		m_done(NULL), m_fail(NULL), m_always(NULL), m_cancel(NULL)
{
}

void DeferredData::resolve()
{
	if(!indeterminate(m_state)) {
		return;
	}
	//keep alive myself, or I'll be deleted in callbacks
	comptr<DeferredData> hold(this);
	m_state = true;
	m_done->invoke_all();
	m_always->invoke_all(m_state);
	detach();
}

void DeferredData::reject()
{
	if(!indeterminate(m_state)) {
		return;
	}
	//keep alive myself, or I'll be deleted in callbacks
	comptr<DeferredData> hold(this);
	m_state = false;
	m_fail->invoke_all();
	m_always->invoke_all(m_state);
	detach();
}

size_t DeferredData::done(const promise_callback_type &func)
{
	if(m_state) {
		func();
	}
	return callback_node_type::insert(&m_done, func);
}

size_t DeferredData::fail(const promise_callback_type &func)
{
	if(!m_state) {
		func();
	}
	return callback_node_type::insert(&m_fail, func);
}

size_t DeferredData::always(const promise_always_type &func)
{
	if(!maybe(m_state)) {
		func(m_state);
	}
	return always_node_type::insert(&m_always, func);
}

void DeferredData::removeDone(size_t handle)
{
	callback_node_type::erase(&m_done, handle);
}

void DeferredData::removeFail(size_t handle)
{
	callback_node_type::erase(&m_fail, handle);
}

void DeferredData::removeAlways(size_t handle)
{
	always_node_type::erase(&m_always, handle);
}

bool DeferredData::isRejected()
{
	return !m_state;
}

bool DeferredData::isResolved()
{
	return m_state;
}

const QVariant& DeferredData::result()
{
	return m_result;
}

void* DeferredData::context()
{
	return m_context;
}

void DeferredData::setContext(void *ctx, cancel_routine cancel)
{
	m_context = ctx;
	m_cancel = cancel;
}

void DeferredData::cancel()
{
	//no reject, we don't want to invoke neither fail nor always
	m_state = false;
	if(m_cancel && m_context) {
		//keep alive myself, or I'll be deleted in callbacks
		comptr<DeferredData> hold(this);
		m_cancel(m_context);
		detach();
	}
}

void DeferredData::detach()
{
	setContext(NULL, NULL);
}
