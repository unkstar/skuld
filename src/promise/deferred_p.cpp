#include "global.h"
#include "misc/leak_detect.h"

#include "deferred_p.h"
#include "deferred_context.h"

//static
DeferredDataPtr DeferredData::shared_indeterminate_deferred(new DeferredData(indeterminate));
//static
DeferredDataPtr DeferredData::shared_true_deferred(new DeferredData(tribool(true)));
//static
DeferredDataPtr DeferredData::shared_false_deferred(new DeferredData(tribool(false)));

//shared static guys only constructor
DeferredData::DeferredData(tribool s)
	: m_state(s), m_activating(false), m_ownContext(false), m_refcount(1),
		m_result(), m_context(NULL), m_done(NULL), m_fail(NULL), m_always(NULL)
{
}

DeferredData::DeferredData()
	: m_state(indeterminate), m_activating(false), m_ownContext(false), m_refcount(1),
		m_result(), m_context(NULL), m_done(NULL), m_fail(NULL), m_always(NULL)
{
	ADD_LEAK_COUNTER_DETAIL(this);
}

//explicit
DeferredData::DeferredData(DeferredContextInterface *ctx)
	: m_state(indeterminate), m_activating(false), m_ownContext(false), m_refcount(1),
		m_result(), m_context(ctx), m_done(NULL), m_fail(NULL), m_always(NULL)
{
	ADD_LEAK_COUNTER_DETAIL(this);
}

//virtual
DeferredData::~DeferredData()
{
	m_done->clear();
	m_fail->clear();
	m_always->clear();

  if(m_ownContext && m_context) {
    delete m_context;
  }

	REMOVE_LEAK_COUNTER_DETAIL(this);
}

void DeferredData::resolve()
{
	//keep alive myself, last reference to me may be gone in a callback
  //this will make up a dangling this pointer
	DeferredDataPtr hold(shared_from_this());
	m_state = true;
	m_activating = true;

	m_done->invoke_all();
	m_always->invoke_all(m_state);

	m_activating = false;
}

void DeferredData::reject()
{
	if(!indeterminate(m_state)) {
		return;
	}
	//keep alive myself, last reference to me may be gone in a callback
  //this will make up a dangling this pointer
	DeferredDataPtr hold(shared_from_this());
	m_state = false;
	m_activating = true;

	m_fail->invoke_all();
	m_always->invoke_all(m_state);

	m_activating = false;
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
	if(!indeterminate(m_state)) {
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

bool DeferredData::isActivating()
{
	return m_activating;
}

tribool DeferredData::state()
{
	return m_state;
}

DeferredContextInterface* DeferredData::context()
{
	return m_context;
}

void DeferredData::setContext(DeferredContextInterface *ctx)
{
	m_context = ctx;
}

void DeferredData::cancel()
{
	//no reject, we don't want to invoke neither fail nor always
	m_state = false;
	if(m_context) {
		//keep alive myself, or I'll be deleted in callbacks
		DeferredDataPtr hold(shared_from_this());
		m_context->cancel_me();
	}
}

tribool DeferredData::reset()
{
	if(m_context) {
		//keep alive myself, or I'll be deleted in callbacks
		DeferredDataPtr hold(shared_from_this());
		m_state = m_context->reset_me();
		if(indeterminate(m_state)) {
			m_result = boost::any();
		}
	}
	return m_state;
}

void DeferredData::resetState(tribool s)
{
	m_state = s;
}

void DeferredData::setResult(const boost::any &r)
{
	m_result = r;
}
const boost::any& DeferredData::result()
{
	return m_result;
}

void DeferredData::setOwnContext(bool owned)
{
  m_ownContext = owned;
}

bool DeferredData::ownContext()
{
  return m_ownContext;
}

