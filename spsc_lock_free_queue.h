#include <atomic>
#include <cassert>
#include <iostream>
#include <vector>
#include <thread>

using namespace std;

template <typename T>
class spsc_lock_free_queue {
public:
  // capacity must be power of two to avoid using modulo operator when calculating the index
  explicit spsc_lock_free_queue(size_t capacity) : m_capacity(capacity), m_buffer(capacity) {
    assert((capacity & (capacity - 1)) == 0 && "capacity must be a power of 2");
  }

  spsc_lock_free_queue(const spsc_lock_free_queue &) = delete;

  spsc_lock_free_queue& operator=(const spsc_lock_free_queue&) = delete;

  bool push(const T& item) {
    size_t tail = m_tail.load(memory_order_relaxed);
    size_t next_tail = (tail + 1) & (m_capacity - 1);
    if (next_tail != m_head.load(memory_order_acquire)) {
      m_buffer[tail] = item;
      m_tail.store(next_tail, memory_order_relased);
      return true;
    }
    return false;
  }

  bool pop(T& item) {
    size_t head = m_head.load(std::memory_order_relaxed);
    if (head == m_tail.load(std::memory_order_acquire)) {
      return false;
    }

    item = m_buffer[head];
    m_head.store((head + 1) & (m_capacity - 1), std::memory_order_release);
    return true;
  }


private:
  const size_t m_capacity;
  vector<T> m_buffer;
  atomic<size_t> m_head{0};
  atomic<size_t> m_tail{0};
};