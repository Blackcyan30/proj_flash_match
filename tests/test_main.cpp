#include <gtest/gtest.h>
#include "flashmatch.hpp"

TEST(MainTest, MainReturnsZero) {
  EXPECT_EQ(0, flashmatch_main(0, nullptr));
}
