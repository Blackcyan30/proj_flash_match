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
#include <system_error>
#include <vector>

#include "flashmatch/matching_engine.hpp"

/**
 * @class stats_t
 * @brief Holds summary statistics for a benchmark run.
 */
struct stats_t {
  /// Total number of orders contained in the dataset.
  std::size_t total_orders = 0;
  /// Number of orders used to warm up the engine before timing.
  std::size_t warmup_orders = 0;
  /// Number of orders processed during the timed portion.
  std::size_t num_orders = 0;
  double mean_latency = 0.0;
  double p50_latency = 0.0;
  double p95_latency = 0.0;
  double p99_latency = 0.0;
  double worst_latency_us = 0.0;
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
  stats_t stats{};

  if (!file.is_open()) {
    std::cout << "Failed to open file: " << filename << std::endl;
    return stats;
  }

  std::size_t warmup_limit = 0;
  std::size_t bench_limit = 0;

  std::string header_line;
  if (!std::getline(file, header_line)) {
    std::cout << "Error: missing header line" << std::endl;
    return stats;
  }

  std::size_t total_rows = 0;
  std::size_t warmup_rows = 0;
  std::string_view header_sv(header_line);
  auto delim = header_sv.find_first_of(", ");
  if (delim == std::string_view::npos) {
    std::cout << "Error: malformed header" << std::endl;
    return stats;
  }
  auto first = header_sv.substr(0, delim);
  auto rest_start = header_sv.find_first_not_of(", ", delim);
  if (rest_start == std::string_view::npos) {
    std::cout << "Error: malformed header" << std::endl;
    return stats;
  }
  auto second = header_sv.substr(rest_start);
  if (std::from_chars(first.data(), first.data() + first.size(), total_rows).ec != std::errc{} ||
      std::from_chars(second.data(), second.data() + second.size(), warmup_rows).ec !=
          std::errc{}) {
    std::cout << "Error: malformed header" << std::endl;
    return stats;
  }

  stats.total_orders = total_rows;
  stats.warmup_orders = warmup_rows;

  warmup_limit = std::min(warmup_rows, total_rows);
  bench_limit = total_rows - warmup_limit;

  fm::MatchingEngine engine;
  std::vector<double> latencies;
  double worst_latency_us = 0.0;


  // Warm up the matching engine.
  std::size_t warmup_ct = 0;
  std::string warmup_line;
  while (warmup_ct < warmup_limit && std::getline(file, warmup_line)) {
    Order new_order;
    if (!parse_order_line(warmup_line, new_order)) {
      std::cout << "Failed to parse line: " << warmup_line << std::endl;
      break;
    }
    // Prefill the matching engine without triggering any matching logic so that
    // subsequent benchmark orders run against a fully populated book.
    engine.insert(new_order);
    ++warmup_ct;
  }

  // Benchmark on the remaining lines.
  std::size_t bench_ct = 0;
  std::string line;
  while (bench_ct < bench_limit && std::getline(file, line)) {
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
    ++bench_ct;
  }

  file.close();

  if (latencies.empty()) {
    return stats;
  }

  stats.num_orders = latencies.size();
  stats.mean_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) /
                       static_cast<double>(latencies.size());

  std::vector<double> temp = latencies;
  auto idx50 = temp.begin() + static_cast<std::size_t>(0.50 * temp.size());
  std::nth_element(temp.begin(), idx50, temp.end());
  stats.p50_latency = *idx50;

  auto idx95 = temp.begin() + static_cast<std::size_t>(0.95 * temp.size());
  std::nth_element(temp.begin(), idx95, temp.end());
  stats.p95_latency = *idx95;

  auto idx99 = temp.begin() + static_cast<std::size_t>(0.99 * temp.size());
  std::nth_element(temp.begin(), idx99, temp.end());
  stats.p99_latency = *idx99;
  stats.worst_latency_us = worst_latency_us;

  return stats;
}

/**
 * @brief
 *
 * @param line
 * @param out
 * @return
 */
bool parse_order_line(std::string_view line, Order &out) {
  std::size_t start = 0, end = 0;
  std::array<std::string_view, 6> tokens;

  for (std::size_t i = 0; i < 5; ++i) {
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
  std::cout << "Total orders:          " << stats.total_orders << "\n";
  std::cout << "Warmup orders:         " << stats.warmup_orders << "\n";
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
