#include <cstdint>
struct Trade {
  std::uint64_t maker_id;
  std::uint64_t taker_id;
  double price;
  std::uint64_t quantity;
};