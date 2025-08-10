#pragma once

#include "lock_free_queue/lock_free_queue.hpp"
#include "types/order.hpp"

// Global lock-free queue used by the order gateway server.
// The queue is defined as an inline variable so it can be
// shared across translation units without requiring a
// separate definition.
inline lfq::Atomic_Queue<Order> g_order_queue{1024};

