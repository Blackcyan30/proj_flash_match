#include <gtest/gtest.h>
#include "flashmatch.hpp"

TEST(MainTest, RunReturnsZero) {
  EXPECT_EQ(0, run_flashmatch());
}
