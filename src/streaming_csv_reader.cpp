#include "../include/flashmatch/high_perf_csv_parser.hpp"
#include <fstream>
#include <string>

// Example: push to your atomic SPSC ring buffer
extern void push_to_ring_buffer(const Order &);

void stream_csv(const std::string &filename) {
  std::ifstream file(filename);
  std::string line;
  Order order;

  while (std::getline(file, line)) {
    if (parse_order_line(line, order)) {
      push_to_ring_buffer(order); // Directly process or enqueue
    }
  }
}
