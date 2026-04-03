add_test([=[Dummy.AlwaysPasses]=]  /home/zeyad/CBs/CodeBROs---CSCE1102-Lab-Project/build/tests/all_tests [==[--gtest_filter=Dummy.AlwaysPasses]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Dummy.AlwaysPasses]=]  PROPERTIES WORKING_DIRECTORY /home/zeyad/CBs/CodeBROs---CSCE1102-Lab-Project/build/tests SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  all_tests_TESTS Dummy.AlwaysPasses)
