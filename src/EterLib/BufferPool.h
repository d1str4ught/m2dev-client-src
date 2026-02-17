#ifndef __INC_ETERLIB_BUFFERPOOL_H__
#define __INC_ETERLIB_BUFFERPOOL_H__

#include <vector>
#include <mutex>
#include <cstdint>

// Buffer pool for file I/O operations
class CBufferPool {
public:
  CBufferPool();
  ~CBufferPool();

  // Get buffer with minimum size
  std::vector<uint8_t> Acquire(size_t minSize);

  // Return buffer to pool
  void Release(std::vector<uint8_t> &&buffer);

  // Get statistics
  size_t GetPoolSize() const;
  size_t GetTotalAllocated() const;
  size_t GetTotalMemoryPooled() const; // Total bytes held in pool

  // Clear pool
  void Clear();

private:
  struct TPooledBuffer {
    std::vector<uint8_t> buffer;
    size_t capacity;

    TPooledBuffer(std::vector<uint8_t> &&buf)
        : buffer(std::move(buf)), capacity(buffer.capacity()) {}
  };

  std::vector<TPooledBuffer> m_pool;
  mutable std::mutex m_mutex;
  size_t m_totalAllocated;

  static const size_t MAX_POOL_SIZE = 64;
  static const size_t MAX_BUFFER_SIZE = 64 * 1024 * 1024;
};

#endif // __INC_ETERLIB_BUFFERPOOL_H__
