#ifndef FLASHMATCH_BENCHMARK_HPP
#define FLASHMATCH_BENCHMARK_HPP

#include <cstddef>
#include <string>

namespace fm {

struct BenchStats {
  std::size_t total_orders = 0;
  std::size_t warmup_orders = 0;
  std::size_t num_orders = 0;
  double mean_latency = 0.0;
  double p50_latency = 0.0;
  double p95_latency = 0.0;
  double p99_latency = 0.0;
  double worst_latency_us = 0.0;
  double total_time_us = 0.0;
};

BenchStats run_bench(const std::string &filename);
double run_engine_bench(const std::string &filename);
void output_stats(const BenchStats &stats);

} // namespace fm

#endif // FLASHMATCH_BENCHMARK_HPP
