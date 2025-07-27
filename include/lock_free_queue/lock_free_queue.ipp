#include <stdexcept>
template <typename T>
lfq::Atomic_Queue<T>::Atomic_Queue(std::uint64_t size)
    : buffer_(nullptr), capacity_(0), head_(0), tail_(0) {
  if (size <= 0) {
    throw std::invalid_argument("Queue capacity must be positive");
  }
  size_ = size;
  capacity_ = size_ + 1;
  buffer_ = std::make_unique<T[]>(capacity_);
}

template <typename T> bool lfq::Atomic_Queue<T>::isEmpty() const {
  return head_.load() == tail_.load();
}

template <typename T> bool lfq::Atomic_Queue<T>::push(T data) {
  auto t = tail_.load(std::memory_order_relaxed);
  auto h = head_.load(std::memory_order_acquire);

  auto nxt = (t + 1) % capacity_;
  if (nxt == h) {
    return false;
  }

  buffer_[t] = data;
  tail_.store(nxt, std::memory_order_release);
  return true;
}

template <typename T> T lfq::Atomic_Queue<T>::pop() {
  auto h = head_.load(std::memory_order_relaxed);
  auto t = tail_.load(std::memory_order_acquire);

  if (h == t) {
    throw std::runtime_error("Queue is empty");
  }

  T to_ret = buffer_[h];
  head_.store((h + 1) % capacity_, std::memory_order_release);
  return to_ret;
}

template <typename T> std::uint64_t lfq::Atomic_Queue<T>::size() const {
  return size_;
}
