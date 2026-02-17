#ifndef __INC_YMIR_ETERLIB_FILELOADERTHREAD_H__
#define __INC_YMIR_ETERLIB_FILELOADERTHREAD_H__

#include <deque>
#include <mutex>
#include "PackLib/PackManager.h"

class CFileLoaderThread {
public:
  typedef struct SData {
    std::string stFileName;
    TPackFile File;
  } TData;

public:
  CFileLoaderThread();
  ~CFileLoaderThread();

  bool Create(void *arg);
  void Shutdown();

public:
  void Request(const std::string &c_rstFileName);
  bool Fetch(TData **ppData);

private:
  void ProcessFile(const std::string &fileName);

private:
  std::deque<TData *> m_pCompleteDeque;
  std::mutex m_CompleteMutex;
  bool m_bShutdowned;
};

#endif