#ifndef __COROUTINE_ANY_H__
#define __COROUTINE_ANY_H__
#pragma once

template<Ty_>
struct is_ptr_size_t
{
	enum {
		value = (sizeof(void*) == sizeof(Ty_));
	};
};

template<Ty_, bool is_ptr_size = is_ptr_size_t<Ty_>::value>
struct any_helper
{
	void* store_any(const Ty_& value)
	{
		return new Ty(value);
	}

	Ty_ any_cast(void *a)
	{
		return *(Ty*)a;
	}

	void destruct_any(void *a)
	{
		delete (Ty*)a;
	}
};

template<Ty_>
struct any_helper<Ty_, true>
{
	void* store_any(const Ty_& value)
	{
		return reinterpret_cast<void*>(value);
	}

	Ty_ any_cast(void *a)
	{
		return reinterpret_cast<Ty_>(a);
	}

	void destruct_any(void *a)
	{
	}
};



#endif //__COROUTINE_ANY_H__
