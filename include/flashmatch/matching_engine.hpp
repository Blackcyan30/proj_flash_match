#ifndef FLASHMATCH_MATCHING_ENGINE_HPP
#define FLASHMATCH_MATCHING_ENGINE_HPP

#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "flashmatch/order_book.hpp"

namespace fm {

class MatchingEngine {
public:
  MatchingEngine() = default;
  // Insert an order without triggering any matching.
  void insert(const Order &order);
  // Queue an order for later processing.
  void add(const Order &order);
  // Process all queued orders and return the trades executed.
  std::vector<Trade> run();
  // Immediately process an order and return the trades executed.
  std::vector<Trade> submit(const Order &order);

private:
  std::unordered_map<std::string, OrderBook> books_;
  std::queue<Order> pending_;
};

} // namespace fm

#endif // FLASHMATCH_MATCHING_ENGINE_HPP
