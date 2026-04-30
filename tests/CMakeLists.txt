add_executable(all_tests
    dummy_test.cpp
    test_auth.cpp
    test_tasks.cpp
    test_messages.cpp
)

target_link_libraries( all_tests PRIVATE GTest::gtest_main shared )

include(GoogleTest)
gtest_discover_tests(all_tests)