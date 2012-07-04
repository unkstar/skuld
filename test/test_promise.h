#include "promise/deferred.h"
#include "promise/promise.h"
#include "promise/deferred_context.h"

namespace
{
	struct rescont {
		rescont()
			:done(false), failed(false), always(false) {}

		void on_done() {done = true;}
		void on_failed() {failed = true;}
		void on_always() {always = true;}

		bool done;
		bool failed;
		bool always;
	};

	class CTX
		: public DeferredContext<int>,
			public rescont
	{
		public:
			CTX() : DeferredContext(), rescont() {}
			virtual void cancel_me() { cancelled = true;}
			void on_done() {
				EXPECT_TRUE(isActivating());
				rescont::done = true;
			}

			void on_failed() {
				EXPECT_TRUE(isActivating());
				rescont::failed = true;
			}

			void on_always() {
				EXPECT_TRUE(isActivating());
				rescont::always = true;
			}

			virtual tribool reset_me() {
				rescont::done = false;
				rescont::failed = false;
				rescont::always = false;
				return boost::logic::indeterminate;
			}

		bool cancelled;
	};

	TEST(Promise, Promise)
	{
		Deferred<void> suc;
		Deferred<void> fail;

		rescont done;
		rescont failed;

		Promise<void> sp = suc;
		sp.done(boost::bind(&rescont::on_done, &done));
		sp.fail(boost::bind(&rescont::on_failed, &done));
		sp.always(boost::bind(&rescont::on_always, &done));

		Promise<void> fp = fail;
		fp.done(boost::bind(&rescont::on_done, &failed));
		fp.fail(boost::bind(&rescont::on_failed, &failed));
		fp.always(boost::bind(&rescont::on_always, &failed));

		suc.resolve();
		EXPECT_TRUE(done.done);
		EXPECT_FALSE(done.failed);
		EXPECT_TRUE(done.always);
		EXPECT_TRUE(sp.isResolved());

		fail.reject();
		EXPECT_FALSE(failed.done);
		EXPECT_TRUE(failed.failed);
		EXPECT_TRUE(failed.always);
		EXPECT_TRUE(fp.isRejected());
	}

	TEST(Promise, DeferredContext)
	{
		CTX suc;

		size_t di, fi, ai;

		Promise<int> sp = suc.promise();
		di = sp.done(boost::bind(&CTX::on_done, &suc));
		fi = sp.fail(boost::bind(&CTX::on_failed, &suc));
		ai = sp.always(boost::bind(&CTX::on_always, &suc));

		EXPECT_NO_THROW(sp.result());
		EXPECT_EQ(int(), sp.result());

		EXPECT_TRUE(boost::logic::indeterminate(sp.state()));

		suc.resolve(1);
		EXPECT_TRUE(suc.rescont::done);
		EXPECT_FALSE(suc.rescont::failed);
		EXPECT_TRUE(suc.rescont::always);
		EXPECT_TRUE(sp.isResolved());
		EXPECT_FALSE(sp.isActivating());
		EXPECT_EQ(1, sp.result());

		EXPECT_TRUE(boost::logic::indeterminate(sp.reset()));

		suc.reject();
		EXPECT_FALSE(suc.rescont::done);
		EXPECT_TRUE(suc.rescont::failed);
		EXPECT_TRUE(suc.rescont::always);
		EXPECT_TRUE(sp.isRejected());
		EXPECT_NO_THROW(sp.result());
		EXPECT_FALSE(sp.isActivating());
		EXPECT_EQ(int(), sp.result());

		EXPECT_TRUE(boost::logic::indeterminate(sp.reset()));

		suc.cancel();
		EXPECT_FALSE(suc.rescont::done);
		EXPECT_FALSE(suc.rescont::failed);
		EXPECT_FALSE(suc.rescont::always);
		EXPECT_TRUE(suc.cancelled);
		EXPECT_TRUE(sp.isRejected());
		EXPECT_FALSE(sp.isActivating());
		EXPECT_NO_THROW(sp.result());
		EXPECT_EQ(int(), sp.result());

		EXPECT_TRUE(boost::logic::indeterminate(sp.reset()));

		sp.removeDone(di);
		sp.removeFail(fi);
		sp.removeAlways(ai);

		suc.resolve(1);
		EXPECT_FALSE(suc.rescont::done);
		EXPECT_FALSE(suc.rescont::failed);
		EXPECT_FALSE(suc.rescont::always);
		EXPECT_FALSE(sp.isActivating());
		EXPECT_NO_THROW(sp.result());
	}
}
