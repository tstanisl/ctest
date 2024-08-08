#ifndef CTEST_H
#define CTEST_H __FILE__

void ctest_fail_test(void);
void ctest_drop_test_loud(const char *fpath, int line);
void ctest_drop_test(void);
void ctest_skip_test(void);
int ctest_failed(void);
int ctest_main(int argc, char * argv[]);

typedef struct ctest {
    const char * name;
    void (*run)(struct ctest *);
    struct ctest * _all_next;
    struct ctest * _res_next;
} ctest;

void ctest_register(ctest *);

#define ctest_entry(ptr, type, member) \
    (type*)((char*)(1 ? ptr : ((type*)0)->member) + offsetof(type, member))

/**
 * @brief Add a test case within a test suite. Parameters must be expanded.
 */
#define CTEST__TEST(tsuite, tcase) \
    static void tsuite ## tcase(void);               \
    static void tsuite ## tcase ## __wrap(ctest *);  \
    __attribute__((constructor))                     \
    static void tsuite ## tcase ##  __init(void) {   \
        static ctest instance = {                    \
            .name = #tsuite "." #tcase,              \
            .run = tsuite ## tcase ## __wrap,        \
        };                                           \
        ctest_register(&instance);                   \
    }                                                \
    static void tsuite ## tcase ## __wrap(ctest*_) { \
        (void)_;                                     \
        tsuite ## tcase ();                          \
    }                                                \
    static void tsuite ## tcase(void)

/**
 * @brief Add a test case within a test suite. Parameters can be macros.
 *
 * @param tsuite a name of the test suite
 * @param tcase a name of the test case with a suite
 */
#define CTEST_TEST(test_suite, test_case) \
    CTEST__TEST(test_suite, test_case)

#define CTEST_FAIL() ctest_drop_test_loud(__FILE__, __LINE__)
#define CTEST_SKIP() ctest_skip_test()
#define CTEST_MAIN() \
int main(int argc, char *argv[]) { \
    return ctest_main(argc, argv); \
}

enum ctest__cmp {
    CTEST__CMP_EQ,
    CTEST__CMP_NE,
    CTEST__CMP_LT,
    CTEST__CMP_LE,
    CTEST__CMP_GT,
    CTEST__CMP_GE,
};

void ctest__cmp_signed(const char *, int, long long signed, const char *,
                       enum ctest__cmp, long long signed, const char *, _Bool);
void ctest__cmp_unsigned(const char *, int, long long unsigned, const char *,
                       enum ctest__cmp, long long unsigned, const char *, _Bool);
void ctest__cmp_double(const char *, int, double, const char *,
                       enum ctest__cmp, double, const char *, _Bool);
void ctest__cmp_str(const char *, int, const char *, const char *,
                       enum ctest__cmp, const char *, const char *, _Bool);
void ctest__cmp_ptr(const char *, int, const volatile void *, const char *,
                       enum ctest__cmp, const volatile void *, const char *, _Bool);
void ctest__check_bool(const char *, int, _Bool, const char *, _Bool, _Bool);

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

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CTEST__CMP_XMACRO(X) \
    X(EQ, ==) \
    X(NE, !=) \
    X(LT,  <) \
    X(LE, <=) \
    X(GT,  >) \
    X(GE, >=) \

static inline const char *ctest__cmp_to_str(enum ctest__cmp cmp) {
    #define X(OP,STR) if (cmp == CTEST__CMP_ ## OP) return #STR;
    CTEST__CMP_XMACRO(X)
    #undef X
    assert(!"Invalid ctest__cmp");
}

void ctest__check_bool(
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
void FNAME(                                       \
    const char *fpath, int lineno,                \
    TYPE a, const char * a_str,                   \
    enum ctest__cmp cmp,                          \
    TYPE b, const char * b_str,                   \
    _Bool drop_on_failure                         \
) {                                               \
    CTEST__CMP_XMACRO(X)                          \
    fprintf(stderr, "%s:%d: Failure\n", fpath, lineno); \
    fprintf(stderr, "Expected: %s %s %s, got\n",  \
        a_str, ctest__cmp_to_str(cmp), b_str);    \
    fprintf(stderr, "  lhs = " FMT "\n", a);      \
    fprintf(stderr, "  rhs = " FMT "\n", b);      \
    fprintf(stderr, "\n");                        \
    if (drop_on_failure) ctest_drop_test();       \
    else                 ctest_fail_test();       \
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


enum ctest_status {
    CTEST_RUNNING,
    CTEST_SUCCESS,
    CTEST_SKIPPED,
    CTEST_FAILURE,
    CTEST_STATUS_COUNT_,
};

static volatile enum ctest_status ctest_status;
static jmp_buf ctest_longjmp_env;

struct ctest_result {
    int count;
    ctest * head;
    ctest ** phead;
};

static struct ctest_result ctest_result[CTEST_STATUS_COUNT_];

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

void ctest_drop_test_loud(const char *fpath, int line) {
    fprintf(stderr, "%s:%d: Failure\n", fpath, line);
    ctest_drop_test(); \
}

void ctest_skip_test(void) {
    if (ctest_status != CTEST_FAILURE)
        ctest_status = CTEST_SKIPPED;
    longjmp(ctest_longjmp_env, 1);
}

int ctest_failed(void) {
    return ctest_status == CTEST_FAILURE;
}

static const char *ctest_status_string[] = {
    [CTEST_RUNNING] = CTEST_COLOR_GREEN  "[ RUNNING    ]" CTEST_COLOR_DEFAULT,
    [CTEST_SKIPPED] = CTEST_COLOR_YELLOW "[    SKIPPED ]" CTEST_COLOR_DEFAULT,
    [CTEST_SUCCESS] = CTEST_COLOR_GREEN  "[    SUCCESS ]" CTEST_COLOR_DEFAULT,
    [CTEST_FAILURE] = CTEST_COLOR_RED    "[    FAILURE ]" CTEST_COLOR_DEFAULT,
};

static ctest * ctest_head;
static ctest ** ctest_tail_p = &ctest_head;

void ctest_register(ctest * test) {
    *ctest_tail_p = test;
    ctest_tail_p = &test->_all_next;
}

static void ctest_run(ctest * t) {

    ctest_status = CTEST_RUNNING;
    fprintf(stderr, "%s: %s\n", ctest_status_string[ctest_status], t->name);

    if (setjmp(ctest_longjmp_env) == 0)
        t->run(t);

    if (ctest_status == CTEST_RUNNING)
        ctest_status = CTEST_SUCCESS;

    struct ctest_result * r = &ctest_result[ctest_status];
    r->count++;
    t->_res_next = 0;
    *r->phead = t;
    r->phead = &t->_res_next;

    fprintf(stderr, "%s: %s\n", ctest_status_string[ctest_status], t->name);
}

static void ctest_list_results(enum ctest_status status, _Bool list) {
    struct ctest_result * res = &ctest_result[status];
    if (res->count == 0)
        return;

    const char * status_str = ctest_status_string[status];
    fprintf(stderr, "%s %d tests.\n", status_str, res->count);
    if (list) {
        for (ctest * node = res->head; node; node = node->_res_next)
            fprintf(stderr, "%s %s\n", status_str, node->name);
    }
    fprintf(stderr, CTEST_COLOR_DEFAULT);
}

int ctest_main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    for (int i = 0; i < CTEST_STATUS_COUNT_; ++i) {
        ctest_result[i].count = 0;
        ctest_result[i].head  = 0;
        ctest_result[i].phead = &ctest_result[i].head;
    }

    for (ctest * node = ctest_head; node; node = node->_all_next)
        ctest_run(node);

    fprintf(stderr, "\n=== SUMMARY ===\n\n");
    ctest_list_results(CTEST_SUCCESS, 0);
    ctest_list_results(CTEST_SKIPPED, 1);
    ctest_list_results(CTEST_FAILURE, 1);

    int failure_cnt = ctest_result[CTEST_FAILURE].count;
    if (failure_cnt == 0) {
        fprintf(stderr, "\nAll tests passed.\n");
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "\n%d tests FAILED.\n", failure_cnt);
        return EXIT_FAILURE;
    }
}

#undef CTEST_IMPLEMENTATION

#endif /* CTEST_IMPLEMENTATION */
