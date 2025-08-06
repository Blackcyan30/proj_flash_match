#include "flashmatch/matching_engine.hpp"
#include <gtest/gtest.h>

using namespace fm;

TEST(MatchingEngineTest, LimitOrderMatching) {
  MatchingEngine me;
  Order ask{1, "AAPL", Side::SELL, 10.0, 100, OrderType::LIMIT};
  me.submit(ask);
  Order bid{2, "AAPL", Side::BUY, 10.0, 50, OrderType::LIMIT};
  auto trades = me.submit(bid);
  ASSERT_EQ(trades.size(), 1u);
  EXPECT_EQ(trades[0].quantity, 50u);
  EXPECT_EQ(trades[0].price, 10.0);
}

TEST(MatchingEngineTest, PartialFillAndRestingOrder) {
  MatchingEngine me;
  Order ask{1, "AAPL", Side::SELL, 10.0, 50, OrderType::LIMIT};
  me.submit(ask);
  Order bid{2, "AAPL", Side::BUY, 10.0, 100, OrderType::LIMIT};
  auto trades = me.submit(bid);
  ASSERT_EQ(trades.size(), 1u);
  EXPECT_EQ(trades[0].quantity, 50u);
  Order ask2{3, "AAPL", Side::SELL, 10.0, 50, OrderType::LIMIT};
  auto trades2 = me.submit(ask2);
  ASSERT_EQ(trades2.size(), 1u);
  EXPECT_EQ(trades2[0].quantity, 50u);
}

TEST(MatchingEngineTest, IOCOrderDoesNotRest) {
  MatchingEngine me;
  Order ask{1, "AAPL", Side::SELL, 10.0, 50, OrderType::LIMIT};
  auto trades = me.submit(ask); // book empty; should rest
  ASSERT_TRUE(trades.empty());
  Order bid{2, "AAPL", Side::BUY, 10.0, 30, OrderType::IOC};
  auto trades2 = me.submit(bid);
  ASSERT_EQ(trades2.size(), 1u);
  EXPECT_EQ(trades2[0].quantity, 30u);
  Order bid2{3, "AAPL", Side::BUY, 10.0, 25, OrderType::LIMIT};
  auto trades3 = me.submit(bid2);
  ASSERT_EQ(trades3.size(), 1u);
  EXPECT_EQ(trades3[0].quantity, 20u);
}

TEST(MatchingEngineTest, DifferentSymbolsDoNotInteract) {
  MatchingEngine me;
  Order ask{1, "AAPL", Side::SELL, 10.0, 100, OrderType::LIMIT};
  me.submit(ask);
  Order bid_other{2, "GOOG", Side::BUY, 10.0, 100, OrderType::LIMIT};
  auto trades = me.submit(bid_other);
  EXPECT_TRUE(trades.empty());
  // Original ask should still be available for AAPL
  Order bid_same{3, "AAPL", Side::BUY, 10.0, 100, OrderType::LIMIT};
  auto trades2 = me.submit(bid_same);
  ASSERT_EQ(trades2.size(), 1u);
  EXPECT_EQ(trades2[0].quantity, 100u);
}

