#include <cstdint>
#include <string>
#include "types/side.hpp"
#include "types/ordertype.hpp"
struct Order {
  std::string symbol;
  std::uint64_t id;
  std::string symbol;
  Side side;
  double price;
  std::uint64_t quantity;
  OrderType type;
};
