add_executable(flashmatch_tests
  test_main.cpp
  test_lock_free_queue.cpp
  test_matching_engine.cpp
  test_order_gateway.cpp
)

target_link_libraries(flashmatch_tests PRIVATE
  flashmatch_lib
  lock_free_queue
  order_gateway_proto
  gtest_main
)
target_compile_options(flashmatch_tests PRIVATE -O3 -march=native)

gtest_discover_tests(flashmatch_tests
  EXTRA_ARGS --gtest_color=yes --gtest_print_time
  DISCOVERY_TIMEOUT 10
)
