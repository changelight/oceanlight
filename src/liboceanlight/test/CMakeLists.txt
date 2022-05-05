add_executable(error_test error_test.cc)
target_link_libraries(error_test liboceanlight gtest_main)
include(GoogleTest)
gtest_discover_tests(error_test)
