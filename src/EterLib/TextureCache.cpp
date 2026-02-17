#include "StdAfx.h"
#include "TextureCache.h"

CTextureCache::CTextureCache(size_t maxMemoryMB)
    : m_maxMemory(maxMemoryMB * 1024 * 1024), m_currentMemory(0), m_hits(0),
      m_misses(0) {}

CTextureCache::~CTextureCache() { Clear(); }

bool CTextureCache::Get(const std::string &filename,
                        TCachedTexture &outTexture) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_cache.find(filename);
  if (it == m_cache.end()) {
    m_misses.fetch_add(1);
    return false;
  }

  // Move to back of LRU (most recently used)
  m_lruList.erase(it->second.second);
  m_lruList.push_back(filename);
  it->second.second = std::prev(m_lruList.end());

  // Copy texture data
  outTexture = it->second.first;

  m_hits.fetch_add(1);
  return true;
}

void CTextureCache::Put(const std::string &filename,
                        const TCachedTexture &texture) {
  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if already cached
  auto it = m_cache.find(filename);
  if (it != m_cache.end()) {
    // Update existing entry
    m_currentMemory -= it->second.first.memorySize;
    m_lruList.erase(it->second.second);
    m_cache.erase(it);
  }

  // Evict if needed
  while (m_currentMemory + texture.memorySize > m_maxMemory &&
         !m_cache.empty()) {
    Evict();
  }

  // Don't cache if too large
  if (texture.memorySize > m_maxMemory / 4) {
    return; // Skip caching huge textures
  }

  // Add to cache
  m_lruList.push_back(filename);
  auto lruIt = std::prev(m_lruList.end());
  m_cache[filename] = {texture, lruIt};
  m_currentMemory += texture.memorySize;
}

void CTextureCache::Clear() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_cache.clear();
  m_lruList.clear();
  m_currentMemory = 0;
}

float CTextureCache::GetHitRate() const {
  size_t hits = m_hits.load();
  size_t misses = m_misses.load();
  size_t total = hits + misses;

  if (total == 0)
    return 0.0f;

  return (float)hits / (float)total;
}

void CTextureCache::Evict() {
  // Remove least recently used (front of list)
  if (m_lruList.empty())
    return;

  const std::string &filename = m_lruList.front();
  auto it = m_cache.find(filename);

  if (it != m_cache.end()) {
    m_currentMemory -= it->second.first.memorySize;
    m_cache.erase(it);
  }

  m_lruList.pop_front();
}
