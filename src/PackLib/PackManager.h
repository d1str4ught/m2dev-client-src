#pragma once
#include <unordered_map>
#include <mutex>

#include "EterBase/Singleton.h"
#include "Pack.h"

class CBufferPool;

class CPackManager : public CSingleton<CPackManager> {
public:
  CPackManager();
  virtual ~CPackManager();

  bool AddPack(const std::string &path);
  bool GetFile(std::string_view path, TPackFile &result);
  bool GetFileWithPool(std::string_view path, TPackFile &result,
                       CBufferPool *pPool);
  bool IsExist(std::string_view path) const;

  void SetPackLoadMode() { m_load_from_pack = true; }
  void SetFileLoadMode() { m_load_from_pack = false; }

  CBufferPool *GetBufferPool() { return m_pBufferPool; }

private:
  void NormalizePath(std::string_view in, std::string &out) const;

private:
  bool m_load_from_pack = true;
  TPackFileMap m_entries;
  CBufferPool *m_pBufferPool;
  mutable std::mutex m_mutex; // Thread safety for parallel pack loading
};
