#include "StdAfx.h"
#include "BufferPool.h"
#include <algorithm>

CBufferPool::CBufferPool() : m_totalAllocated(0) {}

CBufferPool::~CBufferPool() { Clear(); }

std::vector<uint8_t> CBufferPool::Acquire(size_t minSize) {
  std::lock_guard<std::mutex> lock(m_mutex);

  size_t bestIndex = SIZE_MAX;
  size_t bestCapacity = SIZE_MAX;

  for (size_t i = 0; i < m_pool.size(); ++i) {
    if (m_pool[i].capacity >= minSize && m_pool[i].capacity < bestCapacity) {
      bestIndex = i;
      bestCapacity = m_pool[i].capacity;

      if (bestCapacity == minSize)
        break;
    }
  }

  if (bestIndex != SIZE_MAX) {
    std::vector<uint8_t> result = std::move(m_pool[bestIndex].buffer);
    m_pool.erase(m_pool.begin() + bestIndex);
    result.clear();
    return result;
  }

  std::vector<uint8_t> newBuffer;
  newBuffer.reserve(minSize);
  m_totalAllocated++;
  return newBuffer;
}

void CBufferPool::Release(std::vector<uint8_t> &&buffer) {
  size_t capacity = buffer.capacity();

  if (capacity == 0 || capacity > MAX_BUFFER_SIZE) {
    return;
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_pool.size() >= MAX_POOL_SIZE) {
    auto smallest =
        std::min_element(m_pool.begin(), m_pool.end(),
                         [](const TPooledBuffer &a, const TPooledBuffer &b) {
                           return a.capacity < b.capacity;
                         });

    if (smallest != m_pool.end() && smallest->capacity < capacity) {
      *smallest = TPooledBuffer(std::move(buffer));
    }
    return;
  }

  m_pool.emplace_back(std::move(buffer));
}

size_t CBufferPool::GetPoolSize() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_pool.size();
}

size_t CBufferPool::GetTotalAllocated() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_totalAllocated;
}

size_t CBufferPool::GetTotalMemoryPooled() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  size_t total = 0;
  for (const auto &buf : m_pool) {
    total += buf.capacity;
  }
  return total;
}

void CBufferPool::Clear() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_pool.clear();
}
