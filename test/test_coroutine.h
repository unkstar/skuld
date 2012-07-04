#include "coroutine/coroutine.h"

namespace
{
	static int i = 0;

	void co() {
		EXPECT_EQ(i, 1);
		++i;
		GetCurrentCoroutine().yield();
		EXPECT_EQ(i, 3);
		++i;
		Promise<> p;
		EXPECT_THROW(await(p), CancelAsyncException);
	}

	void dummy() {
	}

	TEST(Coroutine, Coroutine)
	{
		EXPECT_EQ(i, 0);
		++i;
		Coroutine cr = async((co));
		EXPECT_FALSE(cr.isCancelled());
		EXPECT_EQ(i, 2);
		EXPECT_TRUE(indeterminate(cr.promise().state()));
		++i;
		cr.run();
		EXPECT_FALSE(cr.isCancelled());
		EXPECT_TRUE(indeterminate(cr.promise().state()));
		EXPECT_EQ(i, 4);
		EXPECT_FALSE(cr.isCancelled());
		cr.promise().cancel();
		EXPECT_TRUE(cr.isCancelled());
		EXPECT_FALSE(cr.promise().state());

		EXPECT_TRUE(async((dummy)).promise().state());
	}
}
