#include "gtest/gtest.h"


#include "test_promise.h"
#include "test_coroutine.h"
#include "test_async.h"



int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, &(argv[0]));

	RUN_ALL_TESTS();
}
