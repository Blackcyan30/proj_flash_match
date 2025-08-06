#include "flashmatch/matching_engine.hpp"
#include <gtest/gtest.h>

using namespace fm;

TEST(MatchingEngineTest, LimitOrderMatching) {
  MatchingEngine me;
  Order ask{1, "AAPL", Side::SELL, 10.0, 100, OrderType::LIMIT};

  me.insert(ask);
  Order bid{2, "AAPL", Side::BUY, 10.0, 50, OrderType::LIMIT};
  me.add(bid);
  auto trades = me.run();

  ASSERT_EQ(trades.size(), 1u);
  EXPECT_EQ(trades[0].quantity, 50u);
  EXPECT_EQ(trades[0].price, 10.0);
}

TEST(MatchingEngineTest, PartialFillAndRestingOrder) {
  MatchingEngine me;
  Order ask{1, "AAPL", Side::SELL, 10.0, 50, OrderType::LIMIT};

  me.insert(ask);
  Order bid{2, "AAPL", Side::BUY, 10.0, 100, OrderType::LIMIT};
  me.add(bid);
  auto trades = me.run();
  ASSERT_EQ(trades.size(), 1u);
  EXPECT_EQ(trades[0].quantity, 50u);
  Order ask2{3, "AAPL", Side::SELL, 10.0, 50, OrderType::LIMIT};
  me.add(ask2);
  auto trades2 = me.run();

  ASSERT_EQ(trades2.size(), 1u);
  EXPECT_EQ(trades2[0].quantity, 50u);
}

TEST(MatchingEngineTest, IOCOrderDoesNotRest) {
  MatchingEngine me;
  Order ask{1, "AAPL", Side::SELL, 10.0, 50, OrderType::LIMIT};

  me.insert(ask); // book empty; should rest
  Order bid{2, "AAPL", Side::BUY, 10.0, 30, OrderType::IOC};
  me.add(bid);
  auto trades2 = me.run();
  ASSERT_EQ(trades2.size(), 1u);
  EXPECT_EQ(trades2[0].quantity, 30u);
  Order bid2{3, "AAPL", Side::BUY, 10.0, 25, OrderType::LIMIT};
  me.add(bid2);
  auto trades3 = me.run();

  ASSERT_EQ(trades3.size(), 1u);
  EXPECT_EQ(trades3[0].quantity, 20u);
}

TEST(MatchingEngineTest, DifferentSymbolsDoNotInteract) {
  MatchingEngine me;
  Order ask{1, "AAPL", Side::SELL, 10.0, 100, OrderType::LIMIT};

  me.insert(ask);
  Order bid_other{2, "GOOG", Side::BUY, 10.0, 100, OrderType::LIMIT};
  me.add(bid_other);
  auto trades = me.run();
  EXPECT_TRUE(trades.empty());
  // Original ask should still be available for AAPL
  Order bid_same{3, "AAPL", Side::BUY, 10.0, 100, OrderType::LIMIT};
  me.add(bid_same);
  auto trades2 = me.run();

  ASSERT_EQ(trades2.size(), 1u);
  EXPECT_EQ(trades2[0].quantity, 100u);
}


TEST(MatchingEngineTest, QueuedOrdersMatchOnRun) {
  MatchingEngine me;
  Order bid{1, "AAPL", Side::BUY, 10.0, 100, OrderType::LIMIT};
  Order ask{2, "AAPL", Side::SELL, 10.0, 100, OrderType::LIMIT};
  me.add(bid);
  me.add(ask);
  auto trades = me.run();
  ASSERT_EQ(trades.size(), 1u);
  EXPECT_EQ(trades[0].quantity, 100u);
  EXPECT_EQ(trades[0].price, 10.0);
}

