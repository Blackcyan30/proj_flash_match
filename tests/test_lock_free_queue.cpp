#include "lock_free_queue/lock_free_queue.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace lfq;

// 1. Basic push and pop
TEST(AtomicQueueTest, PushPopSingleElement) {
  Atomic_Queue<int> q(4);
  EXPECT_TRUE(q.push(100));
  EXPECT_EQ(q.pop(), 100);
}

// 2. Check isEmpty() on new queue
TEST(AtomicQueueTest, InitiallyEmpty) {
  Atomic_Queue<int> q(4);
  EXPECT_TRUE(q.isEmpty());
}

// 3. Push and pop then check isEmpty()
TEST(AtomicQueueTest, EmptyAfterPushPop) {
  Atomic_Queue<int> q(4);
  q.push(1);
  q.pop();
  EXPECT_TRUE(q.isEmpty());
}

// 4. Pop from empty queue throws
TEST(AtomicQueueTest, PopFromEmptyThrows) {
  Atomic_Queue<int> q(4);
  EXPECT_THROW(q.pop(), std::runtime_error);
}

// 5. Push to full queue fails
TEST(AtomicQueueTest, PushToFullFails) {
  Atomic_Queue<int> q(3);
  EXPECT_TRUE(q.push(1));
  EXPECT_TRUE(q.push(2));
  EXPECT_TRUE(q.push(3));
  EXPECT_FALSE(q.push(4));
}

// 6. FIFO ordering
TEST(AtomicQueueTest, FifoOrder) {
  Atomic_Queue<int> q(5);
  q.push(1);
  q.push(2);
  q.push(3);
  EXPECT_EQ(q.pop(), 1);
  EXPECT_EQ(q.pop(), 2);
  EXPECT_EQ(q.pop(), 3);
}

// 7. Wrap around test
TEST(AtomicQueueTest, WrapAround) {
  Atomic_Queue<int> q(3);
  q.push(1);
  q.push(2);
  q.pop();
  q.push(3);
  EXPECT_EQ(q.pop(), 2);
  EXPECT_EQ(q.pop(), 3);
}

// 8. Push negative numbers
TEST(AtomicQueueTest, PushNegative) {
  Atomic_Queue<int> q(2);
  EXPECT_TRUE(q.push(-10));
  EXPECT_EQ(q.pop(), -10);
}

// 9. Use with std::string
TEST(AtomicQueueTest, PushString) {
  Atomic_Queue<std::string> q(2);
  EXPECT_TRUE(q.push("hello"));
  EXPECT_EQ(q.pop(), "hello");
}

// 10. Use with custom struct
struct TestStruct {
  int id;
  std::string name;
  bool operator==(const TestStruct &other) const {
    return id == other.id && name == other.name;
  }
};

TEST(AtomicQueueTest, PushCustomStruct) {
  Atomic_Queue<TestStruct> q(2);
  TestStruct t{1, "Talha"};
  EXPECT_TRUE(q.push(t));
  EXPECT_EQ(q.pop(), t);
}

// 11. Move-only type (unique_ptr)
// TEST(AtomicQueueTest, PushMoveOnly) {
//   Atomic_Queue<std::unique_ptr<int>> q(2);
//   EXPECT_TRUE(q.push(std::make_unique<int>(42)));
//   auto val = q.pop();
//   EXPECT_EQ(*val, 42);
// }

// 12. Stress test single-threaded
TEST(AtomicQueueTest, StressSingleThreaded) {
  const int N = 10000;
  Atomic_Queue<int> q(N + 1);
  for (int i = 0; i < N; ++i)
    EXPECT_TRUE(q.push(i));
  for (int i = 0; i < N; ++i)
    EXPECT_EQ(q.pop(), i);
}

// 13. Multi-threaded push/pop
TEST(AtomicQueueTest, MultiThreaded) {
  constexpr int N = 10000;
  Atomic_Queue<int> q(N + 1);
  std::thread producer([&]() {
    for (int i = 0; i < N; ++i) {
      while (!q.push(i)) {
      }
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
  for (int i = 0; i < N; ++i)
    EXPECT_EQ(result[i], i);
}

// 14. Capacity must be positive
TEST(AtomicQueueTest, ZeroCapacityThrows) {
  EXPECT_THROW(Atomic_Queue<int> q(0), std::invalid_argument);
}

// 15. isEmpty with move-only types
TEST(AtomicQueueTest, EmptyMoveOnly) {
  Atomic_Queue<std::unique_ptr<int>> q(2);
  EXPECT_TRUE(q.isEmpty());
}

// 16. Pushing nullptr unique_ptr
// TEST(AtomicQueueTest, PushNullUniquePtr) {
//   Atomic_Queue<std::unique_ptr<int>> q(2);
//   std::unique_ptr<int> nullPtr;
//   EXPECT_TRUE(q.push(std::move(nullPtr)));
//   auto popped = q.pop();
//   EXPECT_EQ(popped, nullptr);
// }

// 17. Push and pop std::pair
TEST(AtomicQueueTest, PushStdPair) {
  Atomic_Queue<std::pair<int, std::string>> q(2);
  std::pair<int, std::string> p{42, "Talha"};
  EXPECT_TRUE(q.push(p));
  EXPECT_EQ(q.pop(), p);
}

// 18. Push and pop bool
TEST(AtomicQueueTest, PushBool) {
  Atomic_Queue<bool> q(2);
  EXPECT_TRUE(q.push(true));
  EXPECT_TRUE(q.push(false));
  EXPECT_EQ(q.pop(), true);
  EXPECT_EQ(q.pop(), false);
}

// 19. Push float and check precision
TEST(AtomicQueueTest, PushFloat) {
  Atomic_Queue<float> q(2);
  EXPECT_TRUE(q.push(3.1415f));
  EXPECT_NEAR(q.pop(), 3.1415f, 1e-5);
}

// 20. Push and pop std::vector<int>
TEST(AtomicQueueTest, PushVectorInt) {
  Atomic_Queue<std::vector<int>> q(2);
  std::vector<int> vec{1, 2, 3};
  EXPECT_TRUE(q.push(vec));
  EXPECT_EQ(q.pop(), vec);
}

// 21. Push and pop std::shared_ptr
TEST(AtomicQueueTest, PushSharedPtr) {
  Atomic_Queue<std::shared_ptr<int>> q(2);
  auto ptr = std::make_shared<int>(10);
  EXPECT_TRUE(q.push(ptr));
  EXPECT_EQ(*q.pop(), 10);
}

// 22. Repeated wrap-around behavior
TEST(AtomicQueueTest, RepeatedWrapAround) {
  Atomic_Queue<int> q(3);
  for (int i = 0; i < 100; ++i) {
    EXPECT_TRUE(q.push(i % 2));
    q.pop();
  }
  EXPECT_TRUE(q.isEmpty());
}

// 23. Stress with move-only type
// TEST(AtomicQueueTest, StressMoveOnly) {
//   const int N = 1000;
//   Atomic_Queue<std::unique_ptr<int>> q(N + 1);
//   for (int i = 0; i < N; ++i)
//     EXPECT_TRUE(q.push(std::make_unique<int>(i)));
//   for (int i = 0; i < N; ++i) {
//     auto val = q.pop();
//     EXPECT_EQ(*val, i);
//   }
// }

// 24. Push and pop strings of varying lengths
TEST(AtomicQueueTest, VaryingStringSizes) {
  Atomic_Queue<std::string> q(4);
  EXPECT_TRUE(q.push("a"));
  EXPECT_TRUE(q.push("abcdef"));
  EXPECT_TRUE(q.push(""));
  EXPECT_EQ(q.pop(), "a");
  EXPECT_EQ(q.pop(), "abcdef");
  EXPECT_EQ(q.pop(), "");
}

// 25. Move struct with std::move
TEST(AtomicQueueTest, MoveStructExplicit) {
  Atomic_Queue<TestStruct> q(2);
  TestStruct ts{99, "MoveTest"};
  EXPECT_TRUE(q.push(std::move(ts)));
  auto popped = q.pop();
  EXPECT_EQ(popped.id, 99);
  EXPECT_EQ(popped.name, "MoveTest");
}

// 26. Pushing and popping nullptr shared_ptr
TEST(AtomicQueueTest, NullSharedPtr) {
  Atomic_Queue<std::shared_ptr<int>> q(1);
  std::shared_ptr<int> nullPtr;
  EXPECT_TRUE(q.push(nullPtr));
  EXPECT_EQ(q.pop(), nullptr);
}

// 27. Push and pop nested containers
TEST(AtomicQueueTest, NestedContainer) {
  Atomic_Queue<std::vector<std::pair<int, std::string>>> q(2);
  std::vector<std::pair<int, std::string>> nested{{1, "one"}, {2, "two"}};
  EXPECT_TRUE(q.push(nested));
  EXPECT_EQ(q.pop(), nested);
}

// 28. Many push-pop cycles
TEST(AtomicQueueTest, PushPopManyCycles) {
  Atomic_Queue<int> q(5);
  for (int i = 0; i < 1000; ++i) {
    EXPECT_TRUE(q.push(i));
    EXPECT_EQ(q.pop(), i);
  }
  EXPECT_TRUE(q.isEmpty());
}

// 29. Stress with std::string
TEST(AtomicQueueTest, StressString) {
  const int N = 1000;
  Atomic_Queue<std::string> q(N + 1);
  for (int i = 0; i < N; ++i)
    EXPECT_TRUE(q.push(std::to_string(i)));
  for (int i = 0; i < N; ++i)
    EXPECT_EQ(q.pop(), std::to_string(i));
}

// 30. Edge case: Push last element, then full
TEST(AtomicQueueTest, FullAtCapacity) {
  Atomic_Queue<int> q(4);
  EXPECT_TRUE(q.push(1));
  EXPECT_TRUE(q.push(2));
  EXPECT_TRUE(q.push(3));
  EXPECT_TRUE(q.push(4));  // 4rd element in size 4 queue is max
  EXPECT_FALSE(q.push(5)); // full
  EXPECT_EQ(q.pop(), 1);
  EXPECT_TRUE(q.push(5)); // now 1 slot is free
  EXPECT_EQ(q.pop(), 2);
  EXPECT_EQ(q.pop(), 3);
  EXPECT_EQ(q.pop(), 4);
  EXPECT_EQ(q.pop(), 5);
  EXPECT_TRUE(q.isEmpty());
}
