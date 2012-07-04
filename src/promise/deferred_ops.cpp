
#include <boost/bind.hpp>
#include "deferred_p.h"
#include "deferred_ops.h"
#include "deferred_base.h"



template<typename Op_>
class UnaryOpDeferredData
	: public DeferredContextInterface,
    public DeferredData
{

		typedef UnaryOpDeferredData<Op_> my_type;

	public:

		UnaryOpDeferredData();
		~UnaryOpDeferredData();

    void init(DeferredDataPtr src);
		void on_always(tribool state);

		virtual void cancel_me();
		virtual tribool reset_me();

	private:
		DeferredDataPtr m_src;
		size_t m_handle;
};

template<typename Op_>
UnaryOpDeferredData<Op_>::UnaryOpDeferredData()
	: DeferredContextInterface(), m_src(), m_handle(0)
{
  //weird, huh?
  this->setContext(this);
}

template<typename Op_>
void UnaryOpDeferredData<Op_>::init(DeferredDataPtr src)
{
	ASSERT(src);
  m_src = src;
	//callback is holding an weak reference, or a cyclic link will kick in
	//and prevent everything in the syntax tree to be recycled
	m_handle = m_src->always(boost::bind(&UnaryOpDeferredData::on_always, this, _1));
	this->on_always(m_src->state());
}


template<typename Op_>
UnaryOpDeferredData<Op_>::~UnaryOpDeferredData()
{
  ASSERT(m_src);
	m_src->removeAlways(m_handle);
  this->setContext(NULL);
}

//virtual
template<typename Op_>
void UnaryOpDeferredData<Op_>::cancel_me()
{
	this->m_src->cancel();
}

//virtual
template<typename Op_>
tribool UnaryOpDeferredData<Op_>::reset_me()
{
	this->resetState(!m_src->reset());
	return state();
}


template<typename Op_>
void UnaryOpDeferredData<Op_>::on_always(tribool state)
{
	ASSERT(m_src);

	state = Op_::apply(state);
	if(state) {
		resolve();
	} if(!state) {
		reject();
	} else {
		//do nothing
	}
}

template<typename Op_>
class BinaryOpDeferredData
	: public DeferredContextInterface,
    public DeferredData
{

		typedef BinaryOpDeferredData<Op_> my_type;

	public:

		BinaryOpDeferredData();
		~BinaryOpDeferredData();

		void init(DeferredDataPtr left, DeferredDataPtr right);
		virtual void cancel_me();
		virtual tribool reset_me();

		void on_always(tribool state);
	private:
		DeferredDataPtr m_left;
		DeferredDataPtr m_right;
		size_t m_hLeft;
		size_t m_hRight;
};

//virtual
template<typename Op_>
void BinaryOpDeferredData<Op_>::cancel_me()
{
	this->m_left->cancel();
	this->m_right->cancel();
}

//virtual
template<typename Op_>
tribool BinaryOpDeferredData<Op_>::reset_me()
{
	this->resetState(Op_::apply(m_left->reset(), m_right->reset()));
	return state();
}

template<typename Op_>
void BinaryOpDeferredData<Op_>::init(DeferredDataPtr left, DeferredDataPtr right)
{
	ASSERT(left && right);

  m_left = left;
  m_right = right;

	//callback is holding an weak reference, or a cyclic link will kick in
	//and prevent everything in the syntax tree to be recycled
	m_hLeft = m_left->always(boost::bind(&BinaryOpDeferredData<Op_>::on_always, this, _1));
	m_hRight = m_right->always(boost::bind(&BinaryOpDeferredData::on_always, this, _1));

	this->on_always(true); //we don't care what is pass
}

template<typename Op_>
BinaryOpDeferredData<Op_>::BinaryOpDeferredData()
	: DeferredContextInterface(), m_left(), m_right(), m_hLeft(0), m_hRight(0)
{
  this->setContext(this);
}

template<typename Op_>
BinaryOpDeferredData<Op_>::~BinaryOpDeferredData()
{
	ASSERT(m_left && m_right);
	m_left->removeAlways(m_hLeft);
	m_right->removeAlways(m_hRight);
  this->setContext(NULL);
}

template<typename Op_>
void BinaryOpDeferredData<Op_>::on_always(tribool /* unused */)
{
	ASSERT(m_left && m_right);
	tribool state = Op_::apply(m_left->state(), m_right->state());
	if(state) {
		resolve();
	} if(!state) {
		reject();
	} else {
		//do nothing
	}
}

struct TriboolNot
{
	static tribool apply(tribool in)
	{
		return !in;
	}
};

struct TriboolAnd
{
	static tribool apply(tribool lhs, tribool rhs)
	{
		return lhs && rhs;
	}
};

struct TriboolOr
{
	static tribool apply(tribool lhs, tribool rhs)
	{
		return lhs || rhs;
	}
};

typedef UnaryOpDeferredData<TriboolNot> NotOpDeferredData;
typedef BinaryOpDeferredData<TriboolAnd> AndOpDeferredData;
typedef BinaryOpDeferredData<TriboolOr> OrOpDeferredData;


DeferredDataPtr promiseNot(DeferredDataPtr in)
{
  boost::shared_ptr<NotOpDeferredData> p(new NotOpDeferredData());
  p->init(in);
  return p;
}

DeferredDataPtr promiseOr(DeferredDataPtr lhs, DeferredDataPtr rhs)
{
	boost::shared_ptr<OrOpDeferredData> p(new OrOpDeferredData());
  p->init(lhs, rhs);
  return p;
}

DeferredDataPtr promiseAnd(DeferredDataPtr lhs, DeferredDataPtr rhs)
{
  boost::shared_ptr<AndOpDeferredData> p(new AndOpDeferredData());
  p->init(lhs, rhs);
  return p;
}
