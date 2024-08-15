#define CTEST_IMPLEMENTATION
#include "ctest.h"

typedef struct {
    int param;
} Fixture;

TEST_F_INIT(Fixture) {
    LOG("Init fixture\n");
    self->param = 42;
}

TEST_F_DROP(Fixture) {
    LOG("Drop fixture\n");
    self->param = -42;
}

TEST_F(Fixture, Test1) {
    EXPECT_EQ(self->param, 42);
}

TEST_F(Fixture, Test2) {
    EXPECT_EQ(self->param, 43);
}

TEST_F(Fixture, Test3) {
    ASSERT_EQ(self->param, 43);
}

typedef int DummyFixture;
TEST_F(DummyFixture, Test1) {
    EXPECT_EQ(2, 2);
}

typedef struct {
    int _;
} SkipFixture;

TEST_F_INIT(SkipFixture) {
    LOG("Init SkipFixture\n");
    SKIP();
}

TEST_F(SkipFixture, Test1) {
    EXPECT_EQ(41, 42); // never executed
}

CTEST_MAIN()

