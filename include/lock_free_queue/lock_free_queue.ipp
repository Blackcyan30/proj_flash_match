#include <stdexcept>

template <typename T>
Atomic_Queue<T>::Atomic_Queue(int N)
    : buffer_(nullptr), capacity_(N), head_(0), tail_(0) {
  if (N <= 0) {
    throw std::invalid_argument("Queue capacity must be positive");
  }
  buffer_ = std::make_unique<T[]>(N);
}

template <typename T> bool Atomic_Queue<T>::isEmpty() const {
  return head_.load() == tail_.load();
}

template <typename T> bool Atomic_Queue<T>::push(T data) {
  auto t = tail_.load(std::memory_order_relaxed);
  auto h = head_.load(std::memory_order_acquire);

  auto nxt = (t + 1) % capacity_.load();
  if (nxt == h) {
    return false;
  }

  buffer_[t] = data;
  tail_.store(nxt, std::memory_order_release);
  return true;
}

template <typename T> T Atomic_Queue<T>::pop() {
  auto h = head_.load(std::memory_order_relaxed);
  auto t = tail_.load(std::memory_order_acquire);

  if (h == t) {
    throw std::runtime_error("Queue is empty");
  }

  T to_ret = buffer_[h];
  head_.store((h + 1) % capacity_.load(), std::memory_order_release);
  return to_ret;
}
