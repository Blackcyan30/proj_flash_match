#include "flashmatch/matching_engine.hpp"

namespace fm {

std::vector<Trade> MatchingEngine::submit(const Order &order) {
  return books_[order.symbol].match(order);
}

} // namespace fm

