#pragma once
#include <atomic>
#include <cstdint>
#include <memory>

template <typename T> class Atomic_Queue {
private:
  std::unique_ptr<T[]> buffer_;
  std::atomic<std::uint64_t> capacity_;
  std::atomic<std::uint64_t> head_;
  std::atomic<std::uint64_t> tail_;

public:
  Atomic_Queue(int N);
  Atomic_Queue(const Atomic_Queue &) = delete;
  ~Atomic_Queue() = default;
  Atomic_Queue &operator=(const Atomic_Queue &) = delete;
  Atomic_Queue(Atomic_Queue &&) = default;
  Atomic_Queue &operator=(Atomic_Queue &&) = default;

  bool push(T data);
  T pop();
  bool isEmpty() const;
};
#include "lock_free_queue.ipp"
