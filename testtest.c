#define CTEST_IMPLEMENTATION

#include "ctest.h"

int fib(int n) {
	if (n <= 1) return n;
	return fib(n - 1) + fib(n - 2);
}

TEST(Fibonacci) {
	ASSERT_EQ(fib(0), 0);
	ASSERT_EQ(fib(1), 1);
	ASSERT_EQ(fib(2), 1);
	ASSERT_EQ(fib(3), 2);
	ASSERT_EQ(fib(4), 3);
	ASSERT_EQ(fib(5), 5);
	ASSERT_EQ(fib(6), 8);
}
