CTEST is a small, header only library for writing tests in plain C.
The framework deliver functionality similar to GoogleTest.

C11 compatible compiler is required for handling generic selection.

Example

```c

#define CTEST_IMPLEMENTATION
#include <ctest.h>

int fib(int n) {
	if (n <= 1) return n;
	return fib(n - 1) + fib(n - 2);
}

TEST(Fibonacci, Basic) {
	EXPECT_EQ(fib(0), 0);
	EXPECT_EQ(fib(1), 1);
	EXPECT_EQ(fib(2), 1);
	EXPECT_EQ(fib(3), 2);
	EXPECT_EQ(fib(4), 3);
	EXPECT_EQ(fib(5), 5);
	EXPECT_EQ(fib(6), 8);
}

CTEST_MAIN()
```
