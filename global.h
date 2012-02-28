#ifndef __COROUTINE_GLOBAL_H__
#define __COROUTINE_GLOBAL_H__
#pragma once

#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif

//from Qt.Network.q_static_assert
namespace {
    template<bool> struct StaticAssertFailed;
    template<> struct StaticAssertFailed<true> { enum { Value = 1 }; };
}

static inline void noop_with_arg(int) {}
#define STATIC_ASSERT(expr)   noop_with_arg(sizeof(StaticAssertFailed< expr >::Value))

//TODO:copy from yycom.h, include it or move to other file later
template <class I> class comptr
{
protected:
	I* _p;
private:

	class _address
	{
		comptr* m_pThis;
		_address(); //disabled
		explicit _address(comptr* pThis) : m_pThis(pThis) {}
		friend class comptr;
	public:
		_address(const _address& r) : m_pThis(r.m_pThis) {}
		_address& operator =(const _address& r) {m_pThis = r.m_pThis; return *this;}
		operator comptr*() const {return m_pThis;}
		operator void*() const {return m_pThis;}
		operator I**() const {ASSERT(! m_pThis->_p); return &m_pThis->_p;}
		operator void**() const {ASSERT(! m_pThis->_p); return (void**)&m_pThis->_p;}
		comptr* _pointer() const {return m_pThis;}
	};

public:
	//construct/destruct
	comptr() : _p(NULL) {}
	comptr(I* p) : _p(p) {if (_p) _p->AddRef();}
	comptr(const comptr& r) : _p(r._p) {if (_p) _p->AddRef();}
	template <class I2>
	comptr(const comptr<I2>& r) : _p(r.get()) {if (_p) _p->AddRef();}
	~comptr() {if (_p) _p->Release();}

	//operator =
	comptr& operator =(I* p) {if (_p != p) {if (_p) _p->Release(); _p = p; if (_p) _p->AddRef();} return *this;}
	comptr& operator =(const comptr& r) {return operator =(r._p);}
	template <class I2>
	comptr& operator =(const comptr<I2>& r) {return operator =(r.get());}

	//attach/detach
	void attach(I* p) {ASSERT(!_p); if (_p) _p->Release(); _p = p;}
	I* detach() {I* t = _p; _p = NULL; return t;}
	void swap(comptr& r) {I* t = _p; _p = r._p; r._p = t;}

	//operator I* (as I*)
	operator I*() const {return _p;}
	//operator -> (as I*)
	I* operator ->() const {ASSERT(_p); return _p;}

	//operator & (as I**)
//#ifndef YY_LINUX
//	I** operator &() {ASSERT(!_p); return &_p;}
//#else
	_address operator &() {return _address(this);}
//#endif

	//explicit extract _p (as I*, equivalent to operator I*)
	I* get() const
		{ return _p; }
	//explicit extract _p's address (as I**, equivalent to operator &)
	I** _ppv()
		{ ASSERT(!_p); return &_p; }
	//explicit extract comptr's address (as comptr*)
	comptr* _this()
		{return this;}
	const comptr* _this() const
		{return this;}

	//valid
	bool isNull() const
		{ return _p == NULL; }

	//compare operators
	bool operator ==(I* p) const
		{ return _p == p; }
	bool operator !=(I* p) const
		{ return _p != p; }
	bool operator <(I* p) const
		{ return _p < p; }
	bool operator >(I* p) const
		{ return _p > p; }
	bool operator <=(I* p) const
		{ return _p <= p; }
	bool operator >=(I* p) const
		{ return _p >= p; }
};

namespace std
{
	template <class I> inline
	void swap(comptr<I>& sp1, comptr<I>& sp2)
		{sp1.swap(sp2);}
}



#endif //__COROUTINE_GLOBAL_H__
