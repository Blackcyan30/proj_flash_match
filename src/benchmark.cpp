/**
 * @file benchmark.cpp
 * @details This file contains the benchmark code for the order book.
 */
#include <algorithm>
#include <array>
#include <charconv>  // for std::from_chars
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

#include "flashmatch/matching_engine.hpp"

namespace {
// Number of orders submitted during warmup before measurements begin.
constexpr std::size_t kWarmupLimit = 20'000'000;
} 

/**
 * @class stats_t
 * @brief
 *
 */
struct stats_t {
  size_t num_orders;
  double mean_latency;
  double p50_latency;
  double p95_latency;
  double p99_latency;
  double worst_latency_us;
};

stats_t run_bench(const std::string &filename);
bool parse_order_line(std::string_view line, Order &out);
void output_stats(const stats_t &stats);

int main(int argc, char *argv[]) {
  std::cout << "Initializing Benchmark !" << std::endl;
  if (argc < 2) {
    std::cout << "Usage: ./orderbook_bench <data_filename>" << std::endl;
    exit(EXIT_FAILURE);
  }
  // stats_t stats1 = run_bench1(argv[1]);
  stats_t stats = run_bench(argv[1]);
  output_stats(stats);
  return 0;
}

/**
 * @brief
 *
 * @param filename
 * @return
 */
stats_t run_bench(const std::string &filename) {
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cout << "Failed to open file: " << filename << std::endl;
    return stats_t{0, 0.0, 0.0, 0.0, 0.0, 0.0};
  }

  std::string line;
  // Discarding the first line in the file as that just contains
  // what values the columns will contain.
  fm::MatchingEngine engine;
  std::vector<double> latencies;
  double worst_latency_us = 0.0;

  // Warm up the matching engine with a fixed number of orders.
  std::size_t warmup_ct = 0;
  std::string warmup_line;
  while (std::getline(file, warmup_line)) {
    Order new_order;
    if (!parse_order_line(warmup_line, new_order)) {
      std::cout << "Failed to parse line: " << warmup_line << std::endl;
      break;
    }
    engine.submit(new_order);
    ++warmup_ct;
    if (warmup_ct == kWarmupLimit) {
      break;
    }
  }

  // start of bench.
  while (std::getline(file, line)) {
    Order new_order;
    if (!parse_order_line(line, new_order)) {
      std::cout << "Failed to parse line: " << line << std::endl;
      break;
    }

    auto start{std::chrono::steady_clock::now()};
    engine.submit(new_order);
    auto finish{std::chrono::steady_clock::now()};

    std::chrono::duration<double, std::micro> time_elapsed{finish - start};
    double latency_us = time_elapsed.count();
    latencies.push_back(latency_us);
    worst_latency_us = std::max(latency_us, worst_latency_us);
  }

  file.close();

  if (latencies.empty()) {
    return stats_t{0, 0.0, 0.0, 0.0, 0.0, 0.0};
  }

  double mean_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) /
                        static_cast<double>(latencies.size());

  std::vector<double> temp = latencies;
  auto idx50 = temp.begin() + static_cast<size_t>(0.50 * temp.size());
  std::nth_element(temp.begin(), idx50, temp.end());
  double p50_latency = *idx50;

  auto idx95 = temp.begin() + static_cast<size_t>(0.95 * temp.size());
  std::nth_element(temp.begin(), idx95, temp.end());
  double p95_latency = *idx95;

  auto idx99 = temp.begin() + static_cast<size_t>(0.99 * temp.size());
  std::nth_element(temp.begin(), idx99, temp.end());
  double p99_latency = *idx99;

  return stats_t{
      latencies.size(), mean_latency, p50_latency, p95_latency, p99_latency, worst_latency_us};
}

/**
 * @brief
 *
 * @param line
 * @param out
 * @return
 */
bool parse_order_line(std::string_view line, Order &out) {
  size_t start = 0, end = 0;
  std::array<std::string_view, 6> tokens;

  for (int i = 0; i < 5; ++i) {
    end = line.find(',', start);
    if (end == std::string_view::npos) {
      return false;
    }
    tokens[i] = line.substr(start, end - start);
    start = end + 1;
  }
  tokens[5] = line.substr(start);  // last token

  std::from_chars(tokens[0].data(), tokens[0].data() + tokens[0].size(), out.id);
  out.symbol = std::string(tokens[1]);
  out.side = (tokens[2] == "BUY") ? Side::BUY : Side::SELL;
  out.price = std::atof(tokens[3].data());
  std::from_chars(tokens[4].data(), tokens[4].data() + tokens[4].size(), out.quantity);
  out.type = (tokens[5] == "IOC") ? OrderType::IOC : OrderType::LIMIT;

  return true;
}

/**
 * @brief
 *
 * @param stats
 */
void output_stats(const stats_t &stats) {
  std::cout << "=== Flashmatch Benchmark ===\n";
  std::cout << "Orders processed:      " << stats.num_orders << "\n";
  std::cout << std::fixed << std::setprecision(1);
  std::cout << "Mean latency:          " << stats.mean_latency << std::endl;
  std::cout << "Median latency:        " << stats.p50_latency << std::endl;
  std::cout << "95th percentile:       " << stats.p95_latency << std::endl;
  std::cout << "99th percentile:       " << stats.p99_latency << std::endl;
  std::cout << "Worst-case latency:    " << stats.worst_latency_us << std::endl;
  // #ifdef __APPLE__
  //   std::cout << "CPU:                   Apple" << std::endl;
  // #else
  std::ifstream cpuinfo("/proc/cpuinfo");
  std::string model;
  std::getline(cpuinfo, model);
  std::cout << "CPU:                   " << model << '\n';
  // #endif
  std::cout << "Compiler flags:        -O3 -march=native" << std::endl;
}
