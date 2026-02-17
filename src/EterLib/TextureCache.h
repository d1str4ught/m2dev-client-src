#ifndef __INC_ETERLIB_TEXTURECACHE_H__
#define __INC_ETERLIB_TEXTURECACHE_H__

#include <unordered_map>
#include <list>
#include <string>
#include <mutex>

// LRU cache for decoded textures
class CTextureCache {
public:
  struct TCachedTexture {
    std::vector<uint8_t> pixels;
    int width;
    int height;
    size_t memorySize;
    std::string filename;
  };

  CTextureCache(size_t maxMemoryMB = 256);
  ~CTextureCache();

  // Get cached texture
  bool Get(const std::string &filename, TCachedTexture &outTexture);

  // Add texture to cache
  void Put(const std::string &filename, const TCachedTexture &texture);

  // Clear cache
  void Clear();

  // Get statistics
  size_t GetMemoryUsage() const { return m_currentMemory; }
  size_t GetMaxMemory() const { return m_maxMemory; }
  size_t GetCachedCount() const { return m_cache.size(); }
  float GetHitRate() const;

private:
  void Evict();

private:
  size_t m_maxMemory;
  size_t m_currentMemory;

  std::list<std::string> m_lruList;
  std::unordered_map<
      std::string, std::pair<TCachedTexture, std::list<std::string>::iterator>>
      m_cache;

  mutable std::mutex m_mutex;
  std::atomic<size_t> m_hits;
  std::atomic<size_t> m_misses;
};

#endif // __INC_ETERLIB_TEXTURECACHE_H__
