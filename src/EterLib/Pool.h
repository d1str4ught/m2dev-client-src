#pragma once

#include "EterBase/Debug.h"
#include <mutex>

template <typename T> class CDynamicPool {
public:
  CDynamicPool() {}

  virtual ~CDynamicPool() { Destroy(); }

  void Clear() { Destroy(); }

  void Destroy() {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    FreeAll();

    for (T *p : m_Chunks)
      ::free(p);

    m_Free.clear();
    m_Data.clear();
    m_Chunks.clear();
  }

  void Create(size_t chunkSize) { m_uChunkSize = chunkSize; }

  template <class... _Types> T *Alloc(_Types &&..._Args) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (m_Free.empty())
      Grow();

    T *p = m_Free.back();
    m_Free.pop_back();
    return new (p) T(std::forward<_Types>(_Args)...);
  }

  void Free(T *p) {
    p->~T();
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_Free.push_back(p);
  }

  void FreeAll() {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    for (T *p : m_Data) {
      if (std::find(m_Free.begin(), m_Free.end(), p) == m_Free.end()) {
        p->~T();
      }
    }
    m_Free = m_Data;
  }

  size_t GetCapacity() {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_Data.size();
  }

protected:
  void Grow() noexcept {
    size_t uChunkSize = m_uChunkSize + m_uChunkSize * m_Chunks.size();

    T *pStart = (T *)::malloc(uChunkSize * sizeof(T));
    m_Chunks.push_back(pStart);

    m_Data.reserve(m_Data.size() + uChunkSize);
    m_Free.reserve(m_Free.size() + uChunkSize);

    for (size_t i = 0; i < uChunkSize; ++i) {
      m_Data.push_back(pStart + i);
      m_Free.push_back(pStart + i);
    }
  }

protected:
  size_t m_uChunkSize = 64;

  std::vector<T *> m_Data;
  std::vector<T *> m_Free;

  std::vector<T *> m_Chunks;
  std::recursive_mutex m_mutex;
};
