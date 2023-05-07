#ifndef CTEST_H
#define CTEST_H __FILE__

#include <stdio.h>

void ctest_run(void(void), const char*);
void ctest_fail_test(void);
_Noreturn void ctest_drop_test(void);
_Noreturn void ctest_skip_test(void);

#define CTEST_TEST(test_name) \
	static void test_name(void);                                                       \
	__attribute__((constructor)) static void test_name ## __init(void) {               \
		ctest_run(test_name, # test_name);                                         \
	}                                                                                  \
	static void test_name(void)

#define CTEST__LOG_FAIL() \
	fprintf(stderr, "%s:%d: Failure\n", __FILE__, __LINE__)

#define CTEST_FAIL() do {  \
	CTEST__LOG_FAIL(); \
	ctest_drop_test(); \
} while(0)

#define CTEST_SKIP() ctest_skip_test()

#define CTEST_ASSERT_TRUE(pred) do {                       \
	if (pred); else {                                  \
		CTEST__LOG_FAIL();                         \
		fprintf(stderr, "  %s is false\n", #pred); \
		ctest_drop_test();                         \
	}                                                  \
} while (0)

#define CTEST_EXPECT_TRUE(pred) do {                       \
	if (pred); else {                                  \
		CTEST__LOG_FAIL();                         \
		fprintf(stderr, "  %s is false\n", #pred); \
		ctest_fail_test();                         \
	}                                                  \
} while (0)

static inline void ctest__print_signed(long long int val) {
	fprintf(stderr, "%lld", val);
}

static inline void ctest__print_unsigned(unsigned long long int val) {
	fprintf(stderr, "%llu", val);
}

static inline void ctest__print_double(double val) {
	fprintf(stderr, "%g", val);
}

static inline void ctest__print_ptr(const void *val) {
	fprintf(stderr, "%p", val);
}

#define CTEST__PRINT(v) _Generic((v) \
	, char: ctest__print_signed \
	, signed char: ctest__print_signed \
	, short: ctest__print_signed \
	, int: ctest__print_signed \
	, long: ctest__print_signed \
	, long long: ctest__print_signed \
	, unsigned char: ctest__print_unsigned \
	, unsigned short: ctest__print_unsigned \
	, unsigned int: ctest__print_unsigned \
	, unsigned long: ctest__print_unsigned \
	, unsigned long long: ctest__print_unsigned \
	, float: ctest__print_double \
	, double: ctest__print_double \
	, default: ctest__print_ptr \
)(v)

#define CTEST__DUMP_CMP(a, b) do { \
	CTEST__LOG_FAIL();                       \
	fprintf(stderr,   "  lhs:");             \
	CTEST__PRINT(a); \
	fprintf(stderr, "\n  rhs:");             \
	CTEST__PRINT(b); \
	fprintf(stderr, "\n");                   \
} while (0)

#define CTEST_ASSERT__CMP(a, cmp, b) do {     \
	if ((a) cmp (b)); else {              \
		CTEST__DUMP_CMP(a, b);        \
		ctest_drop_test();            \
	}                                     \
} while (0)

#define CTEST_EXPECT__CMP(a, cmp, b) do {     \
	if ((a) cmp (b)); else {              \
		CTEST__DUMP_CMP(a, b);        \
		ctest_fail_test();            \
	}                                     \
} while (0)

#define CTEST_ASSERT_EQ(a, b) CTEST_ASSERT__CMP(a, ==, b)
#define CTEST_EXPECT_EQ(a, b) CTEST_EXPECT__CMP(a, ==, b)

#ifndef CTEST_NO_SHORT_NAMES
#  define ASSERT_TRUE CTEST_ASSERT_TRUE
#  define EXPECT_TRUE CTEST_EXPECT_TRUE
#  define ASSERT_EQ   CTEST_ASSERT_EQ
#  define EXPECT_EQ   CTEST_EXPECT_EQ
#  define FAIL        CTEST_FAIL
#  define SKIP        CTEST_SKIP
#  define TEST        CTEST_TEST
#endif

#endif // CTEST_H


#ifdef CTEST_IMPLEMENTATION

#include <setjmp.h>
#include <stdlib.h>

enum ctest_status {
	CTEST_RUNNING,
	CTEST_SUCCESS,
	CTEST_SKIPPED,
	CTEST_FAILURE,
};

static volatile enum ctest_status ctest_status;
static jmp_buf ctest_longjmp_env;

static int ctest_result_count[4];

#define CTEST_COLOR_RED     "\033[0;31m"
#define CTEST_COLOR_GREEN   "\033[0;32m"
#define CTEST_COLOR_YELLOW  "\033[0;33m"
#define CTEST_COLOR_DEFAULT "\033[0m"

static const char *ctest_status_string[] = {
	[CTEST_RUNNING] = CTEST_COLOR_GREEN  "[ RUNNING ]" CTEST_COLOR_DEFAULT,
	[CTEST_SKIPPED] = CTEST_COLOR_YELLOW "[ SKIPPED ]" CTEST_COLOR_DEFAULT,
	[CTEST_SUCCESS] = CTEST_COLOR_GREEN  "[ SUCCESS ]" CTEST_COLOR_DEFAULT,
	[CTEST_FAILURE] = CTEST_COLOR_RED    "[ FAILURE ]" CTEST_COLOR_DEFAULT,
};

static void ctest_dump_status(const char *test_name) {
	fprintf(stderr, "%s: %s\n", ctest_status_string[ctest_status], test_name);
}

void ctest_drop_test(void) {
	ctest_status = CTEST_FAILURE;
	longjmp(ctest_longjmp_env, 1);
}

void ctest_fail_test(void) {
	ctest_status = CTEST_FAILURE;
}

void ctest_skip_test(void) {
	if (ctest_status != CTEST_FAILURE)
		ctest_status = CTEST_SKIPPED;
	longjmp(ctest_longjmp_env, 1);
}

void ctest_run(void test_fun(void), const char *test_name) {

	ctest_status = CTEST_RUNNING;
	ctest_dump_status(test_name);

	if (setjmp(ctest_longjmp_env) == 0)
		test_fun();

	if (ctest_status == CTEST_RUNNING)
		ctest_status = CTEST_SUCCESS;

	ctest_result_count[ctest_status]++;
	ctest_dump_status(test_name);
}

int main() {
	int success_cnt = ctest_result_count[CTEST_SUCCESS];
	int failure_cnt = ctest_result_count[CTEST_FAILURE];
	int skipped_cnt = ctest_result_count[CTEST_SKIPPED];

	fprintf(stderr, "\n=== SUMMARY ===\n\n");
	fprintf(stderr, "%s%d tests PASSED.\n" CTEST_COLOR_DEFAULT,
	        success_cnt > 0 ? CTEST_COLOR_GREEN : "", success_cnt);
	fprintf(stderr, "%s%d tests SKIPPED.\n" CTEST_COLOR_DEFAULT,
	        skipped_cnt > 0 ? CTEST_COLOR_YELLOW : "", skipped_cnt);
	fprintf(stderr, "%s%d tests FAILED.\n" CTEST_COLOR_DEFAULT,
	        failure_cnt > 0 ? CTEST_COLOR_RED : "", failure_cnt);
	if (failure_cnt == 0) {
		fprintf(stderr, "\nAll tests passed.\n");
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}

#undef CTEST_IMPLEMENTATION
#endif
