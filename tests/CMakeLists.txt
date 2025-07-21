add_executable(flashmatch_tests
  test_main.cpp
  test_lock_free_queue.cpp
)

target_link_libraries(flashmatch_tests PRIVATE
  flashmatch_lib
  lock_free_queue
  gtest_main
)
target_compile_options(flashmatch_tests PRIVATE -O3 -march=native)

add_test(NAME flashmatch_tests COMMAND flashmatch_tests)
