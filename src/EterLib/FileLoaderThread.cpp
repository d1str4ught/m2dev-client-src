#include "StdAfx.h"
#include "EterBase/Stl.h"
#include "PackLib/PackManager.h"
#include "FileLoaderThread.h"
#include "ResourceManager.h"
#include "GameThreadPool.h"

CFileLoaderThread::CFileLoaderThread() : m_bShutdowned(false) {}

CFileLoaderThread::~CFileLoaderThread() { Shutdown(); }

bool CFileLoaderThread::Create(void *arg) {
  // Modern implementation doesn't need explicit thread creation
  // The global CGameThreadPool handles threading
  m_bShutdowned = false;
  return true;
}

void CFileLoaderThread::Shutdown() {
  m_bShutdowned = true;

  // Clear any pending completed items
  {
    std::lock_guard<std::mutex> lock(m_CompleteMutex);
    stl_wipe(m_pCompleteDeque);
  }
}

void CFileLoaderThread::Request(const std::string &c_rstFileName) {
  if (m_bShutdowned)
    return;

  // Enqueue file loading to the global thread pool
  CGameThreadPool *pThreadPool = CGameThreadPool::InstancePtr();
  if (pThreadPool) {
    pThreadPool->Enqueue(
        [this, c_rstFileName]() { ProcessFile(c_rstFileName); });
  } else {
    // Fallback to synchronous loading if thread pool not available
    ProcessFile(c_rstFileName);
  }
}

bool CFileLoaderThread::Fetch(TData **ppData) {
  std::lock_guard<std::mutex> lock(m_CompleteMutex);

  if (m_pCompleteDeque.empty())
    return false;

  *ppData = m_pCompleteDeque.front();
  m_pCompleteDeque.pop_front();

  return true;
}

void CFileLoaderThread::ProcessFile(const std::string &fileName) {
  if (m_bShutdowned)
    return;

  TData *pData = new TData;
  pData->File.clear();
  pData->stFileName = fileName;

  CPackManager::instance().GetFile(pData->stFileName, pData->File);

  // Add to completed queue
  {
    std::lock_guard<std::mutex> lock(m_CompleteMutex);
    m_pCompleteDeque.push_back(pData);
  }

  Sleep(g_iLoadingDelayTime);
}
