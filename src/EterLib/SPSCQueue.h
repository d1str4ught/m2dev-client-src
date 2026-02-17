#ifndef __INC_ETERLIB_SPSCQUEUE_H__
#define __INC_ETERLIB_SPSCQUEUE_H__

#include <atomic>
#include <vector>
#include <cassert>

// Lock-free queue for single producer/consumer pairs
template <typename T> class SPSCQueue {
public:
  explicit SPSCQueue(size_t capacity)
      : m_capacity(capacity + 1) // +1 to distinguish full from empty
        ,
        m_buffer(m_capacity), m_head(0), m_tail(0) {
    assert(capacity > 0);
  }

  ~SPSCQueue() {}

  // Push item (returns false if full)
  bool Push(const T &item) {
    const size_t head = m_head.load(std::memory_order_relaxed);
    const size_t next_head = (head + 1) % m_capacity;

    if (next_head == m_tail.load(std::memory_order_acquire))
      return false; // Queue is full

    m_buffer[head] = item;
    m_head.store(next_head, std::memory_order_release);
    return true;
  }

  // Pop item (returns false if empty)
  bool Pop(T &item) {
    const size_t tail = m_tail.load(std::memory_order_relaxed);

    if (tail == m_head.load(std::memory_order_acquire))
      return false; // Queue is empty

    item = m_buffer[tail];
    m_tail.store((tail + 1) % m_capacity, std::memory_order_release);
    return true;
  }

  // Check if empty
  bool IsEmpty() const {
    return m_tail.load(std::memory_order_acquire) ==
           m_head.load(std::memory_order_acquire);
  }

  // Get queue size
  size_t Size() const {
    const size_t head = m_head.load(std::memory_order_acquire);
    const size_t tail = m_tail.load(std::memory_order_acquire);

    if (head >= tail)
      return head - tail;
    else
      return m_capacity - tail + head;
  }

private:
  const size_t m_capacity;
  std::vector<T> m_buffer;

  alignas(64) std::atomic<size_t> m_head;
  alignas(64) std::atomic<size_t> m_tail;
};

#endif // __INC_ETERLIB_SPSCQUEUE_H__
