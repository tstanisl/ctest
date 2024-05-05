#define CTEST_IMPLEMENTATION

#include "ctest.h"

int fib(int n) {
	if (n <= 1) return n;
	return fib(n - 1) + fib(n - 2);
}

TEST(Fibonacci, Basic) {
	ASSERT_EQ(fib(0), 0);
	ASSERT_EQ(fib(1), 1);
	ASSERT_EQ(fib(2), 1);
	ASSERT_EQ(fib(3), 2);
	ASSERT_EQ(fib(4), 3);
	ASSERT_EQ(fib(5), 5);
	ASSERT_EQ(fib(6), 8);
}

TEST(Fibonacci, Fail) {
	EXPECT_EQ(1+1,1);
	EXPECT_EQ(1+1,2);
	ASSERT_EQ(2+2,4);
	ASSERT_EQ(2+2,5);
}

TEST(Fibonacci, Skip) {
	SKIP();
}

TEST(Types, Basic) {
	EXPECT_EQ('a', 'a');
	char c = 'c';
	EXPECT_EQ(c, c);
	EXPECT_EQ(1u, 1u);
	EXPECT_EQ(1u, 1.0);
	float f = 1;
	EXPECT_EQ(&c, (char*)&f);
}
