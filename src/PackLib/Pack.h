#pragma once
#include <string>
#include <mio/mmap.hpp>

#include "config.h"

class CBufferPool;

class CPack : public std::enable_shared_from_this<CPack> {
public:
  CPack() = default;
  ~CPack() = default;

  bool Load(const std::string &path);
  const std::vector<TPackFileEntry> &GetIndex() const { return m_index; }

  bool GetFile(const TPackFileEntry &entry, TPackFile &result);
  bool GetFileWithPool(const TPackFileEntry &entry, TPackFile &result,
                       CBufferPool *pPool);

private:
  void DecryptData(uint8_t *data, size_t len, const uint8_t *nonce);

  TPackFileHeader m_header;
  std::vector<TPackFileEntry> m_index;
  mio::mmap_source m_file;
};
