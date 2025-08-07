/**
 * @file benchmark.cpp
 * @details This file contains the benchmark code for the order book.
 */
#include <array>
#include <charconv>  // for std::from_chars
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

#include "flashmatch/matching_engine.hpp"

/**
 * @class stats_t
 * @brief
 *
 */
struct stats_t {
  size_t num_orders;
  double mean_latency;
  double p99_latency;
  double worst_latency_us;
};

// Configuration for histogram
constexpr int BIN_WIDTH_US = 1;       // bin size in microseconds
constexpr int MAX_LATENCY_US = 5000;  // cover up to 5000 µs
constexpr int NUM_BINS = MAX_LATENCY_US / BIN_WIDTH_US;

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
    return stats_t{0, 0.0, 0.0, 0.0};
  }
  std::string line;
  [[maybe_unused]] size_t total_samples = 0;
  size_t warmup_limit = 0;
  if (std::getline(file, line)) {
    auto comma_pos = line.find(',');
    if (comma_pos != std::string::npos) {
      std::from_chars(line.data(), line.data() + comma_pos, total_samples);
      std::from_chars(line.data() + comma_pos + 1,
                      line.data() + line.size(), warmup_limit);
    }
  }

  fm::MatchingEngine engine;
  size_t ct = 0;
  std::chrono::duration<double, std::micro> total_latencies_us{0};
  double worst_latency_us = 0.0;
  std::array<size_t, NUM_BINS> histogram{};

  // Warmup for order book
  size_t warmup_ct = 0;
  std::string warmup_line;
  while (warmup_ct < warmup_limit && std::getline(file, warmup_line)) {
    Order new_order;
    if (!parse_order_line(warmup_line, new_order)) {
      std::cout << "Failed to parse line: " << warmup_line << std::endl;
      exit(EXIT_FAILURE);
    }
    engine.insert(new_order);
    ++warmup_ct;
  }

  // start of bench.
  std::cout << "Starting Benchmark!" << std::endl;
  while (std::getline(file, line)) {
    Order new_order;
    if (!parse_order_line(line, new_order)) {
      std::cout << "Failed to parse line: " << line << std::endl;
      exit(EXIT_FAILURE);
    }

    auto start{std::chrono::steady_clock::now()};
    engine.submit(new_order);
    auto finish{std::chrono::steady_clock::now()};

    std::chrono::duration<double, std::micro> time_elapsed{finish - start};

    total_latencies_us += time_elapsed;
    double latency_us = time_elapsed.count();
    worst_latency_us = std::max(latency_us, worst_latency_us);
    ++ct;

    int bin_index = static_cast<int>(latency_us / BIN_WIDTH_US);
    if (bin_index >= NUM_BINS) {
      bin_index = NUM_BINS - 1;
    }
    ++histogram[bin_index];
  }

  file.close();

  double mean_latency = total_latencies_us.count() / ct;
  size_t threshold = static_cast<size_t>(ct * 0.99);
  size_t cumulative = 0;
  int p99_bin = NUM_BINS - 1;

  for (int i = 0; i < NUM_BINS; ++i) {
    cumulative += histogram[i];
    if (cumulative >= threshold) {
      p99_bin = i;
      break;
    }
  }
  double p99_latency = p99_bin * BIN_WIDTH_US;

  return stats_t{ct, mean_latency, p99_latency, worst_latency_us};
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
  std::cout << "Mean latency:        " << stats.mean_latency << std::endl;
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
