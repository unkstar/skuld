#ifndef __COROUTINE_DEFERRED_P_H__
#define __COROUTINE_DEFERRED_P_H__
#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/any.hpp>

#include "../global.h"
#include "deferred_base.h"


//warn myself and all who wanna modify this file:
//PREMATURE OPTIMIZATION IS ROOT OF ALL EVILS
//													--by D. E. Knuth

class DeferredContextInterface;

class DW_COROUTINE_EXPORT DeferredData
  : public boost::enable_shared_from_this<DeferredData>
{
	public:
		static DeferredDataPtr shared_indeterminate_deferred;
		static DeferredDataPtr shared_true_deferred;
		static DeferredDataPtr shared_false_deferred;

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

			void clear()
			{
				CallbackNode *n = this;
				while(n) {
					CallbackNode *t = n->next;
					delete n;
					n = t;
				}
			}
		};

	public:
		typedef boost::function<void()> promise_callback_type;
		typedef boost::function<void(tribool)> promise_always_type;

		typedef CallbackNode<promise_callback_type> callback_node_type;
		typedef CallbackNode<promise_always_type> always_node_type;


		DeferredData();
		explicit DeferredData(tribool s);
		explicit DeferredData(DeferredContextInterface *ctx);
		virtual ~DeferredData();

		void resolve();
		void reject();
		void cancel();
		tribool reset();

		size_t done(const promise_callback_type &func);
		size_t fail(const promise_callback_type &func);
		size_t always(const promise_always_type &func);

		void removeDone(size_t handle);
		void removeFail(size_t handle);
		void removeAlways(size_t handle);

		DeferredContextInterface* context();
		void setContext(DeferredContextInterface *ctx);

		void setOwnContext(bool owned);
		bool ownContext();

		void setResult(const boost::any &r);
		const boost::any& result();


		//XXX:do we need this?
		//void notify(const promise_notify_type&);

		void resetState(tribool s);
		tribool state();

		bool isRejected();
		bool isResolved();

		bool isActivating();

	protected:
		tribool															m_state;
		bool																m_activating;
		bool																m_ownContext;
		int																	m_refcount;
		boost::any													m_result;

		DeferredContextInterface						*m_context;
		callback_node_type									*m_done;
		callback_node_type									*m_fail;
		always_node_type										*m_always;
};
#endif //__COROUTINE_DEFERRED_P_H__
