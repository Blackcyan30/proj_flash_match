#include <cstdint>

#include "types/side.hpp"
struct Order {
  std::string symbol;
  std::uint64_t id;
  Side side;
  double price;
  std::uint64_t quantity;
  OrderType type;
};