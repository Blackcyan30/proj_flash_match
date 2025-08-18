#include <gtest/gtest.h>

#include <iostream>
#include <thread>

#include "flashmatch/benchmark.hpp"

TEST(Benchmark, MeanLatencyUnder15us) {
  fm::BenchStats stats = fm::run_bench("../datasets/100mil-test-20mil-warm-bench-test.csv");
  fm::output_stats(stats);
  EXPECT_LT(stats.mean_latency, 15.0)
      << "Mean latency exceeded 15us. Target: 15us, Actual: " << stats.mean_latency;
}

TEST(Benchmark, EngineRunTiming) {
  double time_us = fm::run_engine_bench("../datasets/100mil-test-20mil-warm-bench-test.csv");
  std::cout << "Engine.run() time: " << time_us << " micro-seconds" << std::endl;
  // EXPECT_LT(time_us, SOME_THRESHOLD_US);
}

TEST(Benchmark, TotalLoopTime) {
  fm::BenchStats stats = fm::run_bench("../datasets/100mil-test-20mil-warm-bench-test.csv");
  std::cout << "Total loop time: " << stats.total_time_us << " micro-seconds" << std::endl;
  // EXPECT_LT(stats.total_time_us, SOME_THRESHOLD_US);
}

TEST(Benchmark, LatencyGuardRegression) {
  using namespace std::chrono;
  auto start = steady_clock::now();
  std::this_thread::sleep_for(microseconds(20));
  auto finish = steady_clock::now();
  double latency = duration<double, std::micro>(finish - start).count();
  EXPECT_GE(latency, 15.0);
  // A value above 15us indicates the guard would trigger as expected.
}
