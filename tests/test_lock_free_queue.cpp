#include "lock_free_queue/lock_free_queue.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <thread>
#include <vector>

TEST(AtomicQueueTest, PushPopSingleElement) {
  Atomic_Queue<int> q(10);
  EXPECT_TRUE(q.push(42));
  EXPECT_EQ(q.pop(), 42);
}

TEST(AtomicQueueTest, QueueEmptyInitially) {
  Atomic_Queue<int> q(5);
  EXPECT_TRUE(q.isEmpty());
}

TEST(AtomicQueueTest, QueueIsEmptyAfterPushAndPop) {
  Atomic_Queue<int> q(5);
  q.push(1);
  q.pop();
  EXPECT_TRUE(q.isEmpty());
}

TEST(AtomicQueueTest, PushToFullQueueFails) {
  Atomic_Queue<int> q(3);
  EXPECT_TRUE(q.push(1));
  EXPECT_TRUE(q.push(2));
  EXPECT_FALSE(q.push(3)); // Third push should fail in a 3-slot queue
}

TEST(AtomicQueueTest, PopFromEmptyThrows) {
  Atomic_Queue<int> q(3);
  EXPECT_THROW(q.pop(), std::runtime_error);
}

TEST(AtomicQueueTest, FIFOOrderPreserved) {
  Atomic_Queue<int> q(5);
  q.push(1);
  q.push(2);
  q.push(3);
  EXPECT_EQ(q.pop(), 1);
  EXPECT_EQ(q.pop(), 2);
  EXPECT_EQ(q.pop(), 3);
}

TEST(AtomicQueueTest, WrapAroundBehavior) {
  Atomic_Queue<int> q(3);
  q.push(1);
  q.push(2);
  q.pop();   // Removes 1
  q.push(3); // Should wrap
  EXPECT_EQ(q.pop(), 2);
  EXPECT_EQ(q.pop(), 3);
}

TEST(AtomicQueueTest, MultiThreadedStressTest) {
  constexpr int N = 10000;
  Atomic_Queue<int> q(N + 1);

  std::thread producer([&]() {
    for (int i = 0; i < N; ++i) {
      while (!q.push(i)) {
      } // Spin until successful
    }
  });

  std::vector<int> result;
  std::thread consumer([&]() {
    for (int i = 0; i < N; ++i) {
      int val;
      while (true) {
        try {
          val = q.pop();
          break;
        } catch (...) {
        }
      }
      result.push_back(val);
    }
  });

  producer.join();
  consumer.join();

  ASSERT_EQ(result.size(), N);
  for (int i = 0; i < N; ++i) {
    EXPECT_EQ(result[i], i);
  }
}
