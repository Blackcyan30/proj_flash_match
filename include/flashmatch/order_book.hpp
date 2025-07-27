#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <vector>

namespace fm {
enum class Side { BUY, SELL };
enum class OrderType { LIMIT, IOC };

struct Order {
  std::uint64_t id;
  Side side;
  double price;
  std::uint64_t quantity;
  OrderType type;
};

struct Trade {
  std::uint64_t maker_id;
  std::uint64_t taker_id;
  double price;
  std::uint64_t quantity;
};

class OrderBook {
private:
  using OrderDeque = std::deque<Order>;
  std::map<double, OrderDeque, std::greater<double>> bids_;
  std::map<double, OrderDeque, std::less<double>> asks_;

public:
  OrderBook() = default;
  // Process incoming order and process trades executed.
  std::vector<Trade> match(Order order);
  // Insert a limit order without matching.
  void insertOrder(const Order &order);
};

} // namespace fm
