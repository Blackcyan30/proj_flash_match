#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <vector>
#include <types/order.hpp>
#include <types/trade.hpp>
#include <types/side.hpp>
#include <types/ordertype.hpp> // Include necessary headers

namespace fm {
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
