#ifndef CTEST_H
#define CTEST_H __FILE__

#include <assert.h>
#include <stdio.h>
#include <string.h>

void ctest_run(void(void), const char*);
void ctest_fail_test(void);
_Noreturn void ctest_drop_test(void);
_Noreturn void ctest_skip_test(void);

/**
 * @brief Add a test case within a test suite. Parameters must be expanded.
 */
#define CTEST__TEST(tsuite, tcase) \
	static void tsuite ## tcase(void);                                          \
	__attribute__((constructor)) static void tsuite ## tcase ##  __init(void) { \
		ctest_run(tsuite ## tcase, #tsuite "." #tcase);                     \
	}                                                                           \
	static void tsuite ## tcase(void)

/**
 * @brief Add a test case within a test suite. Parameters can be macros.
 *
 * @param tsuite a name of the test suite
 * @param tcase a name of the test case with a suite
 */
#define CTEST_TEST(test_suite, test_case) \
	CTEST__TEST(test_suite, test_case)

#define CTEST__LOG_FAIL() \
	fprintf(stderr, "%s:%d: Failure\n", __FILE__, __LINE__)

#define CTEST_FAIL() do {  \
	CTEST__LOG_FAIL(); \
	ctest_drop_test(); \
} while(0)

#define CTEST_SKIP() ctest_skip_test()

#define CTEST__CMP_XMACRO(X) \
	X(LT,  <)            \
	X(LE, <=)            \
	X(EQ, ==)            \
	X(NE, !=)            \
	X(GE, >=)            \
	X(GT,  >)            \

enum ctest__cmp {
	#define X(name, op)  CTEST__CMP_ ## name,
	CTEST__CMP_XMACRO(X)
	#undef X
};

static inline const char *ctest__cmp_to_str(enum ctest__cmp cmp) {
	#define X(OP,STR) if (cmp == CTEST__CMP_ ## OP) return #STR;
	CTEST__CMP_XMACRO(X)
	#undef X
	assert(!"Invalid ctest__cmp");
}

//static inline void ctest__cmp_unsigned() { }
//static inline void ctest__cmp_double() { }
//static inline void ctest__cmp_str() { }
//static inline void ctest__cmp_ptr() { }

#if 0
static inline void ctest__cmp_signed(const char *fname, int lineno,
                                     signed long long a, const char *a_str,
                                     enum ctest__cmp cmp,
				     signed long long b, const char *b_str,
				     _Bool drop_on_failure
) {
	_Bool passed;
	const char * op_str;
	switch (cmp) {
	#define X(name, op) case CTEST__CMP_ ## name: passed = a op b; op_str = #op; break;
	CTEST__CMP_XMACRO(X)
	#undef X
	}

	if (passed) return;
	fprintf(stderr, "%s:%d: Failure\nExpected: (%s) %s (%s), got\n  lhs=%lld\n  rhs=%lld\n",
	        fname, lineno, a_str, op_str, b_str, a, b);
	if (drop_on_failure) ctest_drop_test();
	else                 ctest_fail_test();
}
#endif

static inline void ctest__check_bool(
	const char *fpath, int lineno,
	_Bool a, const char * a_str,
	_Bool b,
	_Bool drop_on_failure
) {
	if (a == b) return;
	fprintf(stderr, "%s:%d: Failure\n", fpath, lineno);
	fprintf(stderr, "Expected: (%s) to be %s\n",
		a_str, b ? "true" : "false");

	if (drop_on_failure) ctest_drop_test();
	else                 ctest_fail_test();
}

#define CTEST__CMP_FUNC_IMPL(FNAME, TYPE, FMT, X) \
static inline void FNAME( \
	const char *fpath, int lineno, \
	TYPE a, const char * a_str, \
	enum ctest__cmp cmp, \
	TYPE b, const char * b_str, \
	_Bool drop_on_failure \
) { \
	CTEST__CMP_XMACRO(X) \
	fprintf(stderr, "%s:%d: Failure\n", fpath, lineno); \
	fprintf(stderr, "Expected: %s %s %s, got\n", \
		a_str, ctest__cmp_to_str(cmp), b_str); \
	fprintf(stderr, "  lhs=" FMT "\n", a); \
	fprintf(stderr, "  rhs=" FMT "\n", b); \
	fprintf(stderr, "\n"); \
	if (drop_on_failure) ctest_drop_test(); \
	else                 ctest_fail_test(); \
}

#define X(name, op) if (cmp == CTEST__CMP_ ## name && a op b) return;
CTEST__CMP_FUNC_IMPL(ctest__cmp_signed,     long long signed, "%lld", X)
CTEST__CMP_FUNC_IMPL(ctest__cmp_unsigned, long long unsigned, "%llu", X)
CTEST__CMP_FUNC_IMPL(ctest__cmp_double,               double,   "%g", X)
CTEST__CMP_FUNC_IMPL(ctest__cmp_ptr,   const volatile void *,   "%p", X)
#undef X
#define X(name, op) if (cmp == CTEST__CMP_ ## name && strcmp(a,b) op 0) return;
CTEST__CMP_FUNC_IMPL(ctest__cmp_str, const char *, "\"%s\"", X)
#undef X


/*
*/

#if 0

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

//#define CTEST_ASSERT_EQ(a, b) CTEST_ASSERT__CMP(a, ==, b)
//#define CTEST_EXPECT_EQ(a, b) CTEST_EXPECT__CMP(a, ==, b)
#endif

//#define CTEST_ASSERT_EQ(a, b) CTEST_ASSERT__CMP(a, ==, b)

#define CTEST_ASSERT_TRUE(pred) \
	ctest__check_bool(__FILE__, __LINE__, (pred), #pred, 1, 1)
#define CTEST_ASSERT_FALSE(pred) \
	ctest__check_bool(__FILE__, __LINE__, (pred), #pred, 0, 1)
#define CTEST_EXPECT_TRUE(pred) \
	ctest__check_bool(__FILE__, __LINE__, (pred), #pred, 1, 0)
#define CTEST_EXPECT_FALSE(pred) \
	ctest__check_bool(__FILE__, __LINE__, (pred), #pred, 0, 0)

#define CTEST__CMP(a, cmp, b, drop_on_fail) \
_Generic(1 ? (a) : (b) \
	, _Bool: ctest__cmp_unsigned \
	, char: ctest__cmp_signed \
	, signed char: ctest__cmp_signed \
	, short: ctest__cmp_signed \
	, int: ctest__cmp_signed \
	, long: ctest__cmp_signed \
	, long long: ctest__cmp_signed \
	, unsigned char: ctest__cmp_unsigned \
	, unsigned short: ctest__cmp_unsigned \
	, unsigned int: ctest__cmp_unsigned \
	, unsigned long: ctest__cmp_unsigned \
	, unsigned long long: ctest__cmp_unsigned \
	, float: ctest__cmp_double \
	, double: ctest__cmp_double \
	, const char*: ctest__cmp_str \
	, char*: ctest__cmp_str \
	, default: ctest__cmp_ptr \
)(__FILE__, __LINE__, a, #a, CTEST__CMP_ ## cmp, b, #b, drop_on_fail)

#define CTEST_EXPECT_EQ(a, b) CTEST__CMP(a, EQ, b, 0)
#define CTEST_ASSERT_EQ(a, b) CTEST__CMP(a, EQ, b, 1)

#define CTEST__STR_CMP(a, cmp, b, drop_on_fail) \
	ctest__cmp_str(__FILE__, __LINE__, a, #a, CTEST__CMP_ ## cmp, b, #b, drop_on_fail)

#define CTEST_EXPECT_STR_EQ(a, b) CTEST__STR_CMP(a, EQ, b, 0)
#define CTEST_ASSERT_STR_EQ(a, b) CTEST__STR_CMP(a, EQ, b, 1)

#define CTEST__PTR_CMP(a, cmp, b, drop_on_fail) \
	ctest__cmp_ptr(__FILE__, __LINE__, a, #a, CTEST__CMP_ ## cmp, b, #b, drop_on_fail)

#define CTEST_EXPECT_PTR_EQ(a, b) CTEST__PTR_CMP(a, EQ, b, 0)
#define CTEST_ASSERT_PTR_EQ(a, b) CTEST__PTR_CMP(a, EQ, b, 1)

#ifndef CTEST_NO_SHORT_NAMES
#  define ASSERT_TRUE  CTEST_ASSERT_TRUE
#  define EXPECT_TRUE  CTEST_EXPECT_TRUE
#  define ASSERT_FALSE CTEST_ASSERT_FALSE
#  define EXPECT_FALSE CTEST_EXPECT_FALSE
#  define ASSERT_EQ   CTEST_ASSERT_EQ
#  define EXPECT_EQ   CTEST_EXPECT_EQ
#  define ASSERT_STR_EQ   CTEST_ASSERT_STR_EQ
#  define EXPECT_STR_EQ   CTEST_EXPECT_STR_EQ
#  define ASSERT_PTR_EQ   CTEST_ASSERT_PTR_EQ
#  define EXPECT_PTR_EQ   CTEST_EXPECT_PTR_EQ
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
	[CTEST_RUNNING] = CTEST_COLOR_GREEN  "[ RUNNING    ]" CTEST_COLOR_DEFAULT,
	[CTEST_SKIPPED] = CTEST_COLOR_YELLOW "[    SKIPPED ]" CTEST_COLOR_DEFAULT,
	[CTEST_SUCCESS] = CTEST_COLOR_GREEN  "[    SUCCESS ]" CTEST_COLOR_DEFAULT,
	[CTEST_FAILURE] = CTEST_COLOR_RED    "[    FAILURE ]" CTEST_COLOR_DEFAULT,
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

#endif /* CTEST_IMPLEMENTATION */
