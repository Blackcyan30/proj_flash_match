#include "flashmatch/matching_engine.hpp"

namespace fm {

void MatchingEngine::insert(const Order &order) {
  books_[order.symbol].insertOrder(order);
}

void MatchingEngine::add(const Order &order) { pending_.push(order); }

std::vector<Trade> MatchingEngine::run() {
  std::vector<Trade> all_trades;
  while (!pending_.empty()) {
    auto order = pending_.front();
    pending_.pop();
    auto trades = submit(order);
    all_trades.insert(all_trades.end(), trades.begin(), trades.end());
  }
  return all_trades;
}

std::vector<Trade> MatchingEngine::submit(const Order &order) {
  return books_[order.symbol].match(order);
}

} // namespace fm
