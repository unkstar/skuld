#ifndef __COROUTINE_DEFERRED_OPS_H__
#define __COROUTINE_DEFERRED_OPS_H__
#pragma once

#include "deferred_p.h"
#include "boost/bind.hpp"

template<typename Op_>
class UnaryOpDeferredData : public DeferredData {

		typedef UnaryOpDeferredData<Op_> my_type;

	public:

		UnaryOpDeferredData(DeferredData* src);
		~UnaryOpDeferredData();

		void on_always(tribool state);

		static void cancel_me(void* ctx);
	private:
		DeferredData* m_src;
		size_t m_handle;
};

//static
template<typename Op_>
void UnaryOpDeferredData<Op_>::cancel_me(void* ctx)
{
	UnaryOpDeferredData<Op_>* pThis = (UnaryOpDeferredData<Op_>*)ctx;
	pThis->m_src->cancel();
}

template<typename Op_>
UnaryOpDeferredData<Op_>::UnaryOpDeferredData(DeferredData* src)
	: DeferredData(), m_src(src), m_handle(NULL)
{
	ASSERT(m_src);
	m_src->AddRef();

	//callback is holding an weak reference, or a cyclic link will kick in
	//and prevent everything in the syntax tree to be recycled
	m_handle = m_src->always(boost::bind(&UnaryOpDeferredData::on_always, this, _1));
	this->setContext(this, my_type::cancel_me);
	this->on_always(m_src->state());
}

template<typename Op_>
UnaryOpDeferredData<Op_>::~UnaryOpDeferredData()
{
	ASSERT(m_src);
	m_src->removeAlways(m_handle);
	m_src->Release();
}

template<typename Op_>
void UnaryOpDeferredData<Op_>::on_always(tribool state)
{
	ASSERT(m_src);

	state = Op_::apply(state);
	if(state) {
		this->resolve();
	} if(!state) {
		this->reject();
	} else {
		//do nothing
	}
}

template<typename Op_>
class BinaryOpDeferredData : public DeferredData {

		typedef BinaryOpDeferredData<Op_> my_type;

	public:

		BinaryOpDeferredData(DeferredData* left, DeferredData* right);
		~BinaryOpDeferredData();

		static void cancel_me(void* ctx);

		void on_always(tribool state);
	private:
		DeferredData* m_left;
		DeferredData* m_right;
		size_t m_hLeft;
		size_t m_hRight;
};

//static
template<typename Op_>
void BinaryOpDeferredData<Op_>::cancel_me(void* ctx)
{
	BinaryOpDeferredData<Op_>* pThis = (BinaryOpDeferredData<Op_>*)ctx;
	pThis->m_left->cancel();
	pThis->m_right->cancel();
}

template<typename Op_>
BinaryOpDeferredData<Op_>::BinaryOpDeferredData(DeferredData* left, DeferredData* right)
	: DeferredData(), m_left(left), m_right(right), m_hLeft(NULL), m_hRight(NULL)
{
	ASSERT(m_left && m_right);
	m_left->AddRef();
	m_right->AddRef();

	//callback is holding an weak reference, or a cyclic link will kick in
	//and prevent everything in the syntax tree to be recycled
	m_hLeft = m_left->always(boost::bind(&BinaryOpDeferredData<Op_>::on_always, this, _1));
	m_hRight = m_right->always(boost::bind(&BinaryOpDeferredData::on_always, this, _1));
	this->setContext(this, my_type::cancel_me);

	this->on_always(true); //we don't care what is pass
}

template<typename Op_>
BinaryOpDeferredData<Op_>::~BinaryOpDeferredData()
{
	ASSERT(m_left && m_right);
	m_left->removeAlways(m_hLeft);
	m_right->removeAlways(m_hRight);
	m_left->Release();
	m_right->Release();
}

template<typename Op_>
void BinaryOpDeferredData<Op_>::on_always(tribool /* unused */)
{
	ASSERT(m_left && m_right);
	tribool state = Op_::apply(m_left->state(), m_right->state());
	if(state) {
		this->resolve();
	} if(!state) {
		this->reject();
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


#endif //__COROUTINE_DEFERRED_OPS_H__
