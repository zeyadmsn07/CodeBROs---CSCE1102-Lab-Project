enable_testing()

add_executable(all_tests
    dummy_test.cpp
)

target_link_libraries(all_tests
    GTest::gtest_main
    shared
)

include(GoogleTest)
gtest_discover_tests(all_tests)

