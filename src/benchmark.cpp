/**
 * @file benchmark.cpp
 * @details This file contains the benchmark code for the order book.
 */
#include "../include/flashmatch/order_book.hpp"
#include <array>
#include <charconv> // for std::from_chars
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

struct stats_t {
  size_t num_orders;
  double mean_latency;
  double p99_latency;
  double worst_latency_us;
};

// Configuration for histogram
constexpr int BIN_WIDTH_US = 1;      // bin size in microseconds
constexpr int MAX_LATENCY_US = 5000; // cover up to 5000 µs
constexpr int NUM_BINS = MAX_LATENCY_US / BIN_WIDTH_US;

// fm::Order parse_csv_line(const std::string &line) { return fm::Order{}; }
stats_t run_bench1(const std::string &filename);
stats_t run_bench2(const std::string &filename);
bool parse_order_line(std::string_view line, fm::Order &out);
void output_stats(const stats_t &stats);

int main(int argc, char *argv[]) {
  std::cout << "Starting benchmark!" << std::endl;
  if (argc < 2) {
    std::cout << "Usage: ./orderbook_bench <data_filename>" << std::endl;
    exit(EXIT_FAILURE);
  }
  // stats_t stats1 = run_bench1(argv[1]);
  stats_t stats2 = run_bench2(argv[1]);
  output_stats(stats2);
  return 0;
}

stats_t run_bench1(const std::string &filename) {
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cout << "Failed to open file: " << filename << std::endl;
  }

  std::string line;
  // discarding the first line in the file as that just contains
  // what values the columns will contain.
  fm::OrderBook ob;
  size_t ct = 0;
  std::chrono::duration<double, std::micro> total_latencies_us{0};
  double worst_latency_us = 0.0;
  std::array<size_t, NUM_BINS> histogram{};
  while (std::getline(file, line)) {
    fm::Order new_order;
    if (!parse_order_line(line, new_order)) {
      std::cout << "Failed to parse line: " << line << std::endl;
      break;
    }

    auto start{std::chrono::steady_clock::now()};
    ob.match(new_order);
    auto finish{std::chrono::steady_clock::now()};
    std::chrono::duration<double, std::micro> time_elapsed{finish - start};
    total_latencies_us += time_elapsed;
    double latency_us = time_elapsed.count();
    worst_latency_us = std::max(latency_us, worst_latency_us);
    ++ct;
    // if (ct == 1)
    // break;
    int bin_index = static_cast<int>(latency_us / BIN_WIDTH_US);
    if (bin_index >= NUM_BINS)
      bin_index = NUM_BINS - 1;
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

stats_t run_bench2(const std::string &filename) {
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cout << "Failed to open file: " << filename << std::endl;
  }

  std::string line;
  // discarding the first line in the file as that just contains
  // what values the columns will contain.
  fm::OrderBook ob;
  size_t ct = 0;
  std::chrono::duration<double, std::micro> total_latencies_us{0};
  double worst_latency_us = 0.0;
  std::array<size_t, NUM_BINS> histogram{};

  // Warmup for order book
  constexpr size_t WARMUP_LIMIT = 20000000;
  size_t warmup_ct = 0;
  std::string warmup_line;
  while (std::getline(file, warmup_line)) {
    fm::Order new_order;
    if (!parse_order_line(warmup_line, new_order)) {
      std::cout << "Failed to parse line: " << warmup_line << std::endl;
      break;
    }
    ob.insertOrder(new_order);
    ++warmup_ct;
    if (warmup_ct == WARMUP_LIMIT)
      break;
  }

  // start of bench.
  while (std::getline(file, line)) {
    fm::Order new_order;
    if (!parse_order_line(line, new_order)) {
      std::cout << "Failed to parse line: " << line << std::endl;
      break;
    }

    auto start{std::chrono::steady_clock::now()};
    ob.match(new_order);
    auto finish{std::chrono::steady_clock::now()};
    std::chrono::duration<double, std::micro> time_elapsed{finish - start};
    total_latencies_us += time_elapsed;
    double latency_us = time_elapsed.count();
    worst_latency_us = std::max(latency_us, worst_latency_us);
    ++ct;
    // if (ct == 1)
    // break;
    int bin_index = static_cast<int>(latency_us / BIN_WIDTH_US);
    if (bin_index >= NUM_BINS)
      bin_index = NUM_BINS - 1;
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

bool parse_order_line(std::string_view line, fm::Order &out) {
  size_t start = 0, end = 0;
  std::array<std::string_view, 5> tokens;

  for (int i = 0; i < 4; ++i) {
    end = line.find(',', start);
    if (end == std::string_view::npos)
      return false;
    tokens[i] = line.substr(start, end - start);
    start = end + 1;
  }
  tokens[4] = line.substr(start); // last token

  // Parse with no allocations
  std::from_chars(tokens[0].data(), tokens[0].data() + tokens[0].size(),
                  out.id);
  out.side = (tokens[1] == "BUY") ? fm::Side::BUY : fm::Side::SELL;
  // std::from_chars(tokens[2].data(), tokens[2].data() + tokens[2].size(),
  //                 out.price);
  out.price = std::atof(tokens[2].data());
  std::from_chars(tokens[3].data(), tokens[3].data() + tokens[3].size(),
                  out.quantity);
  out.type = (tokens[4] == "IOC") ? fm::OrderType::IOC : fm::OrderType::LIMIT;

  return true;
}

void output_stats(const stats_t &stats) {
  std::cout << "=== Flashmatch Benchmark ===\n";
  std::cout << "Orders processed:      " << stats.num_orders << "\n";
  std::cout << std::fixed << std::setprecision(1);
  std::cout << "Mean latency:        " << stats.mean_latency << std::endl;
  std::cout << "99th percentile:       " << stats.p99_latency << std::endl;
  std::cout << "Worst-case latency:    " << stats.worst_latency_us << std::endl;
}
