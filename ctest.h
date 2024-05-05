#ifndef CTEST_H
#define CTEST_H __FILE__

#include <assert.h>
#include <stdio.h>
#include <string.h>

void ctest_run(void(void), const char*);
void ctest_failure(const char *fpath, int line);
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


#define CTEST_FAIL() ctest_failure(__FILE__, __LINE__)
#define CTEST_SKIP() ctest_skip_test()

enum ctest__cmp {
	CTEST__CMP_EQ,
	CTEST__CMP_NE,
	CTEST__CMP_LT,
	CTEST__CMP_LE,
	CTEST__CMP_GT,
	CTEST__CMP_GE,
};

#define CTEST__CMP_XMACRO(X) \
	X(EQ, ==)            \
	X(NE, !=)            \
	X(LT,  <)            \
	X(LE, <=)            \
	X(GT,  >)            \
	X(GE, >=)            \

static inline const char *ctest__cmp_to_str(enum ctest__cmp cmp) {
	#define X(OP,STR) if (cmp == CTEST__CMP_ ## OP) return #STR;
	CTEST__CMP_XMACRO(X)
	#undef X
	assert(!"Invalid ctest__cmp");
}

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
#define CTEST_EXPECT_NE(a, b) CTEST__CMP(a, NE, b, 0)
#define CTEST_ASSERT_NE(a, b) CTEST__CMP(a, NE, b, 1)
#define CTEST_EXPECT_LT(a, b) CTEST__CMP(a, LT, b, 0)
#define CTEST_ASSERT_LT(a, b) CTEST__CMP(a, LT, b, 1)
#define CTEST_EXPECT_LE(a, b) CTEST__CMP(a, LE, b, 0)
#define CTEST_ASSERT_LE(a, b) CTEST__CMP(a, LE, b, 1)
#define CTEST_EXPECT_GT(a, b) CTEST__CMP(a, GT, b, 0)
#define CTEST_ASSERT_GT(a, b) CTEST__CMP(a, GT, b, 1)
#define CTEST_EXPECT_GE(a, b) CTEST__CMP(a, GE, b, 0)
#define CTEST_ASSERT_GE(a, b) CTEST__CMP(a, GE, b, 1)

#define CTEST__STR_CMP(a, cmp, b, drop_on_fail) \
	ctest__cmp_str(__FILE__, __LINE__, a, #a, CTEST__CMP_ ## cmp, b, #b, drop_on_fail)

#define CTEST_EXPECT_STR_EQ(a, b) CTEST__STR_CMP(a, EQ, b, 0)
#define CTEST_ASSERT_STR_EQ(a, b) CTEST__STR_CMP(a, EQ, b, 1)
#define CTEST_EXPECT_STR_NE(a, b) CTEST__STR_CMP(a, NE, b, 0)
#define CTEST_ASSERT_STR_NE(a, b) CTEST__STR_CMP(a, NE, b, 1)
#define CTEST_EXPECT_STR_LT(a, b) CTEST__STR_CMP(a, LT, b, 0)
#define CTEST_ASSERT_STR_LT(a, b) CTEST__STR_CMP(a, LT, b, 1)
#define CTEST_EXPECT_STR_LE(a, b) CTEST__STR_CMP(a, LE, b, 0)
#define CTEST_ASSERT_STR_LE(a, b) CTEST__STR_CMP(a, LE, b, 1)
#define CTEST_EXPECT_STR_GT(a, b) CTEST__STR_CMP(a, GT, b, 0)
#define CTEST_ASSERT_STR_GT(a, b) CTEST__STR_CMP(a, GT, b, 1)
#define CTEST_EXPECT_STR_GE(a, b) CTEST__STR_CMP(a, GE, b, 0)
#define CTEST_ASSERT_STR_GE(a, b) CTEST__STR_CMP(a, GE, b, 1)

#ifndef CTEST_NO_SHORT_NAMES
#  define ASSERT_TRUE  CTEST_ASSERT_TRUE
#  define EXPECT_TRUE  CTEST_EXPECT_TRUE
#  define ASSERT_FALSE CTEST_ASSERT_FALSE
#  define EXPECT_FALSE CTEST_EXPECT_FALSE
#  define ASSERT_EQ   CTEST_ASSERT_EQ
#  define EXPECT_EQ   CTEST_EXPECT_EQ
#  define ASSERT_NE   CTEST_ASSERT_NE
#  define EXPECT_NE   CTEST_EXPECT_NE
#  define ASSERT_LT   CTEST_ASSERT_LT
#  define EXPECT_LT   CTEST_EXPECT_LT
#  define ASSERT_LE   CTEST_ASSERT_LE
#  define EXPECT_LE   CTEST_EXPECT_LE
#  define ASSERT_GT   CTEST_ASSERT_GT
#  define EXPECT_GT   CTEST_EXPECT_GT
#  define ASSERT_GE   CTEST_ASSERT_GE
#  define EXPECT_GE   CTEST_EXPECT_GE
#  define ASSERT_STR_EQ   CTEST_ASSERT_STR_EQ
#  define EXPECT_STR_EQ   CTEST_EXPECT_STR_EQ
#  define ASSERT_STR_NE   CTEST_ASSERT_STR_NE
#  define EXPECT_STR_NE   CTEST_EXPECT_STR_NE
#  define ASSERT_STR_LT   CTEST_ASSERT_STR_LT
#  define EXPECT_STR_LT   CTEST_EXPECT_STR_LT
#  define ASSERT_STR_LE   CTEST_ASSERT_STR_LE
#  define EXPECT_STR_LE   CTEST_EXPECT_STR_LE
#  define ASSERT_STR_GT   CTEST_ASSERT_STR_GT
#  define EXPECT_STR_GT   CTEST_EXPECT_STR_GT
#  define ASSERT_STR_GE   CTEST_ASSERT_STR_GE
#  define EXPECT_STR_GE   CTEST_EXPECT_STR_GE
#  define ASSERT_STR_EQ   CTEST_ASSERT_STR_EQ
#  define EXPECT_STR_EQ   CTEST_EXPECT_STR_EQ
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

void ctest_drop_test(void) {
	ctest_status = CTEST_FAILURE;
	longjmp(ctest_longjmp_env, 1);
}

void ctest_fail_test(void) {
	ctest_status = CTEST_FAILURE;
}

void ctest_failure(const char *fpath, int line) {
	fprintf(stderr, "%s:%d: Failure\n", fpath, line);
	ctest_drop_test(); \
}

void ctest_skip_test(void) {
	if (ctest_status != CTEST_FAILURE)
		ctest_status = CTEST_SKIPPED;
	longjmp(ctest_longjmp_env, 1);
}

static const char *ctest_status_string[] = {
	[CTEST_RUNNING] = CTEST_COLOR_GREEN  "[ RUNNING    ]" CTEST_COLOR_DEFAULT,
	[CTEST_SKIPPED] = CTEST_COLOR_YELLOW "[    SKIPPED ]" CTEST_COLOR_DEFAULT,
	[CTEST_SUCCESS] = CTEST_COLOR_GREEN  "[    SUCCESS ]" CTEST_COLOR_DEFAULT,
	[CTEST_FAILURE] = CTEST_COLOR_RED    "[    FAILURE ]" CTEST_COLOR_DEFAULT,
};

void ctest_run(void test_fun(void), const char *test_name) {

	ctest_status = CTEST_RUNNING;
	fprintf(stderr, "%s: %s\n", ctest_status_string[ctest_status], test_name);

	if (setjmp(ctest_longjmp_env) == 0)
		test_fun();

	if (ctest_status == CTEST_RUNNING)
		ctest_status = CTEST_SUCCESS;

	ctest_result_count[ctest_status]++;
	fprintf(stderr, "%s: %s\n", ctest_status_string[ctest_status], test_name);
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
