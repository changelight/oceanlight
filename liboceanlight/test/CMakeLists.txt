enable_testing()
add_executable(lol_utility_test lol_utility_test.cc)
target_link_libraries(lol_utility_test liboceanlight GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(lol_utility_test)
