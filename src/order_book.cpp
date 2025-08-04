#include "flashmatch/order_book.hpp"
// #include <algorithm>

namespace fm {

void OrderBook::insertOrder(const Order &order) {
  if (order.side == Side::BUY) {
    bids_[order.price].push_back(order);
  } else {
    asks_[order.price].push_back(order);
  }
}

std::vector<Trade> OrderBook::match(Order order) {
  std::vector<Trade> trades;
  // If order is to buy.
  if (order.side == Side::BUY) {
    while (order.quantity > 0 && !asks_.empty()) {
      auto it = asks_.begin();
      // To check if buyer's price is more than the lowest the seller is
      // selling. Now since in the map it is ordered from lowest to largest the
      // first value is automatically the lowest price asked by the seller to
      // sell.
      if (it->first > order.price)
        break;

      auto &deque = it->second;
      while (order.quantity > 0 && !deque.empty()) {
        Order &order_seller = deque.front();
        std::uint64_t traded = std::min(order.quantity, order_seller.quantity);
        trades.push_back(
            Trade{order_seller.id, order.id, order_seller.price, traded});
        order.quantity -= traded;
        order_seller.quantity -= traded;
        if (order_seller.quantity == 0) {
          deque.pop_front();
        }
      }
      if (deque.empty()) {
        asks_.erase(it);
      }
    }
  } else {
    while (order.quantity > 0 && !bids_.empty()) {
      auto it = bids_.begin();
      // To check if the seller's ask is greater than the
      // highest buyer's bid, if so then cancel matching
      // since than there is no trade to be made.
      if (order.price > it->first)
        break;

      auto &deque = it->second;
      while (order.quantity > 0 && !deque.empty()) {
        Order &order_buyer = deque.front();
        std::uint64_t traded = std::min(order.quantity, order_buyer.quantity);
        trades.push_back(
            Trade{order_buyer.id, order.id, order_buyer.price, traded});
        order.quantity -= traded;
        order_buyer.quantity -= traded;
        if (order_buyer.quantity == 0) {
          deque.pop_front();
        }
      }
      if (deque.empty()) {
        bids_.erase(it);
      }
    }
  }
  if (order.quantity > 0 && order.type == OrderType::LIMIT) {
    insertOrder(order);
  }
  return trades;
}

} // namespace fm
