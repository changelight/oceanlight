#include <gtest/gtest.h>
#include <liboceanlight/lol_engine.hpp>
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

/*
const char** glfwGetRequiredInstanceExtensions(uint32_t* count)
{
	static const char* extensions = "VK_KHR_surface";
	return &extensions;
}

extern int foo();

int __wrap_foo()
{
	return 1 + 2;
}
*/

TEST(basic_tests, create_instance_creates_instance)
{
	//EXPECT_EQ(foo(), 3);
	/*liboceanlight::engine::instance_data inst_data {};
	liboceanlight::engine::debug_messenger_data dbg_data {};

	EXPECT_NO_THROW(
		liboceanlight::engine::create_instance(inst_data, dbg_data));
	EXPECT_TRUE(inst_data.vulkan_instance);*/
}
