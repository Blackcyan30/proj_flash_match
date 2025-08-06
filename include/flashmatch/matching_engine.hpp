#ifndef FLASHMATCH_MATCHING_ENGINE_HPP
#define FLASHMATCH_MATCHING_ENGINE_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "flashmatch/order_book.hpp"

namespace fm {

class MatchingEngine {
public:
  MatchingEngine() = default;
  std::vector<Trade> submit(const Order &order);

private:
  std::unordered_map<std::string, OrderBook> books_;
};

} // namespace fm

#endif // FLASHMATCH_MATCHING_ENGINE_HPP
