// Benchmark tests for Flashmatch

#include "flashmatch/benchmark.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <thread>

// Location of the benchmark dataset within the repository root.
class BenchmarkTest : public ::testing::Test {
protected:
  static constexpr const char *kData = "ob_100mil_bench_20mil_warm.csv";

  static std::filesystem::path dataset_path() {
    return std::filesystem::path(__FILE__).parent_path() / ".." / "datasets" /
           kData;
  }

  static void SetUpTestSuite() {
    auto path = dataset_path();
    if (!std::filesystem::exists(path)) {
      std::string cmd =
          "(cd " +
          (std::filesystem::path(__FILE__).parent_path() / ".." / "datasets")
              .string() +
          " && python3 generate_orderbook.py)";
      int result = std::system(cmd.c_str());
      ASSERT_EQ(result, 0) << "Dataset generation failed with code " << result;
    }
    ASSERT_TRUE(std::filesystem::exists(path))
        << "Dataset missing after generation";
  }
};

TEST_F(BenchmarkTest, MeanLatencyUnder15us) {
  fm::BenchStats stats = fm::run_bench(dataset_path().string());
  ASSERT_GT(stats.num_orders, 0);
  ASSERT_LT(stats.p99_latency, 40000);
  fm::output_stats(stats);
  ASSERT_LT(stats.mean_latency, 15.0)
      << "Mean latency exceeded 15us. Target: 15us, Actual: " << stats.mean_latency;
}

TEST_F(BenchmarkTest, EngineRunTiming) {
  double time_us = fm::run_engine_bench(dataset_path().string());
  ASSERT_GT(time_us, 0) << "Engine benchmark did not run";
  std::cout << "Engine.run() time: " << time_us << " micro-seconds" << std::endl;
}

TEST_F(BenchmarkTest, TotalLoopTime) {
  fm::BenchStats stats = fm::run_bench(dataset_path().string());
  ASSERT_GT(stats.num_orders, 0);
  ASSERT_LT(stats.p99_latency, 40000);
  std::cout << "Total loop time: " << stats.total_time_us << " micro-seconds" << std::endl;
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
