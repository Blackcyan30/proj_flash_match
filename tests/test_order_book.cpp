#include "flashmatch/order_book.hpp"
#include <gtest/gtest.h>

using namespace fm;

TEST(OrderBookTest, LimitOrderMatching) {
  OrderBook ob;
  Order ask{1, Side::SELL, 10.0, 100, OrderType::LIMIT};
  ob.insertOrder(ask);
  Order bid{2, Side::BUY, 10.0, 50, OrderType::LIMIT};
  auto trades = ob.match(bid);
  ASSERT_EQ(trades.size(), 1u);
  EXPECT_EQ(trades[0].quantity, 50u);
  EXPECT_EQ(trades[0].price, 10.0);
}

TEST(OrderBookTest, PartialFillAndRestingOrder) {
  OrderBook ob;
  Order ask{1, Side::SELL, 10.0, 50, OrderType::LIMIT};
  ob.insertOrder(ask);
  Order bid{2, Side::BUY, 10.0, 100, OrderType::LIMIT};
  auto trades = ob.match(bid);
  ASSERT_EQ(trades.size(), 1u);
  EXPECT_EQ(trades[0].quantity, 50u);
  // Remaining quantity should rest on bids
  Order ask2{3, Side::SELL, 10.0, 50, OrderType::LIMIT};
  auto trades2 = ob.match(ask2);
  ASSERT_EQ(trades2.size(), 1u);
  EXPECT_EQ(trades2[0].quantity, 50u);
}

TEST(OrderBookTest, IOCOrderDoesNotRest) {
  OrderBook ob;
  Order ask{1, Side::SELL, 10.0, 50, OrderType::LIMIT};
  auto trades = ob.match(ask); // book empty; should rest
  ASSERT_TRUE(trades.empty());
  Order bid{2, Side::BUY, 10.0, 30, OrderType::IOC};
  auto trades2 = ob.match(bid);
  ASSERT_EQ(trades2.size(), 1u);
  EXPECT_EQ(trades2[0].quantity, 30u);
  // leftover ask should now have 20
  Order bid2{3, Side::BUY, 10.0, 25, OrderType::LIMIT};
  auto trades3 = ob.match(bid2);
  ASSERT_EQ(trades3.size(), 1u);
  EXPECT_EQ(trades3[0].quantity, 20u);
  // 5 remaining from bid2 should rest on bids
}
