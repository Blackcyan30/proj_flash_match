add_executable(flashmatch_tests test_main.cpp)

target_link_libraries(flashmatch_tests PRIVATE
  flashmatch_lib
  gtest_main
)

add_test(NAME flashmatch_tests COMMAND flashmatch_tests)
