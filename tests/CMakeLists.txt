add_executable(all_tests
    dummy_test.cpp
    test_auth.cpp
    test_tasks.cpp
    test_messages.cpp
)

# Explicitly link BOTH gtest and gtest_main, plus the shared library
target_link_libraries(all_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    shared
)

# Fixes symbol visibility issues specific to MinGW/Windows compilation
if(WIN32)
    target_compile_definitions(all_tests PRIVATE GTEST_LINKED_AS_SHARED_LIBRARY=1)
endif()

include(GoogleTest)
gtest_discover_tests(all_tests)