#ifndef __COROUTINE_DEFERRED_P_H__
#define __COROUTINE_DEFERRED_P_H__
#pragma once

#include "boost/logic/tribool.hpp"
#include "boost/function.hpp"
#include <QVariant>
#include "global.h"

using boost::logic::tribool;
BOOST_TRIBOOL_THIRD_STATE(maybe)

//PREMATURE OPTIMIZATION IS ROOT OF ALL EVILS
//													--by D. E. Knuth

class DeferredData
{
	public:
		static DeferredData shared_indeterminate_deferred;
		static DeferredData shared_true_deferred;
		static DeferredData shared_false_deferred;

		//poor guy's single link list
		template<typename Fn_>
		struct CallbackNode
		{
			CallbackNode *next;
			Fn_ func;

			static size_t insert(CallbackNode **head, const Fn_ &f)
			{
				CallbackNode *n = new CallbackNode;
				n->func = f;
				n->next = *head;
				*head = n;
				return (size_t)n;
			}

			static void erase(CallbackNode **head, size_t handle)
			{
				CallbackNode *n = *head;
				CallbackNode **pn = head;
				CallbackNode *tar = (CallbackNode*)handle;
				if(!handle)return;
				while(n) {
					if(tar == n) {
						*pn = n->next;
						delete tar;
						return;
					}
					pn = &n->next;
					n = n->next;
				}
			}

			void invoke_all()
			{
				CallbackNode *n = this;
				while(n) {
					CallbackNode *t = n->next;
					n->func();
					n = t;
				}
			}

			void invoke_all(tribool res)
			{
				CallbackNode *n = this;
				while(n) {
					CallbackNode *t = n->next;
					n->func(res);
					n = t;
				}
			}
		};

	public:
		typedef boost::function<void()> promise_callback_type;
		typedef boost::function<void(tribool)> promise_always_type;

		typedef CallbackNode<promise_callback_type> callback_node_type;
		typedef CallbackNode<promise_always_type> always_node_type;

		typedef void (*cancel_routine)(void*);


		DeferredData();
		explicit DeferredData(tribool s);
		virtual ~DeferredData() {}

		//XXX:maybe we shall add AtomicOps here to make DeferredData much more widely usable?
		void AddRef() { ++m_refcount;}
		void Release() { if(--m_refcount == 0) delete this;}

		void resolve();
		void reject();
		void cancel();

		void detach();

		size_t done(const promise_callback_type &func);
		size_t fail(const promise_callback_type &func);
		size_t always(const promise_always_type &func);

		void removeDone(size_t handle);
		void removeFail(size_t handle);
		void removeAlways(size_t handle);

		const QVariant& result();

		template<typename Ty_>
		void setResult(const Ty_ &val)
		{
			m_result.setValue(val);
		}

		void* context();
		void setContext(void *ctx, cancel_routine cancel);


		//XXX:do we need this?
		//void notify(const promise_notify_type&);

		bool isRejected();
		bool isResolved();

		tribool state() { return m_state; }


	protected:
		tribool															m_state;
		int																	m_refcount;
		QVariant														m_result;
		void																*m_context;

		callback_node_type									*m_done;
		callback_node_type									*m_fail;
		always_node_type										*m_always;
		cancel_routine											m_cancel;
};
#endif //__COROUTINE_DEFERRED_P_H__
