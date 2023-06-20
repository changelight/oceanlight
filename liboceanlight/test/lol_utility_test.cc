#include <gtest/gtest.h>
#include <liboceanlight/lol_utility.hpp>

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions)
{
	// Expect two strings not to be equal.
	EXPECT_STRNE("hello", "world");
	// Expect equality.
	EXPECT_EQ(7 * 6, 42);
}

TEST(basic_tests, test_func_returns_a_plus_b)
{
	int a = 42, b = 12;
	EXPECT_EQ(liboceanlight::test_func(a, b), a + b);
}
