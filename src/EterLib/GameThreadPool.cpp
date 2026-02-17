#include "StdAfx.h"
#include "GameThreadPool.h"

CGameThreadPool::CGameThreadPool()
    : m_bShutdown(false), m_bInitialized(false) {}

CGameThreadPool::~CGameThreadPool() { Destroy(); }

void CGameThreadPool::Initialize(int iWorkerCount) {
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);

  if (m_bInitialized.load(std::memory_order_acquire)) {
    TraceError("CGameThreadPool::Initialize - Already initialized!");
    return;
  }

  // Determine worker count
  if (iWorkerCount <= 0) {
    iWorkerCount = static_cast<int>(std::thread::hardware_concurrency());
    if (iWorkerCount <= 0)
      iWorkerCount = 4; // Fallback to 4 workers
  }

  // Clamp worker count to reasonable range
  iWorkerCount = std::max(2, std::min(16, iWorkerCount));

  Tracef("CGameThreadPool::Initialize - Creating %d worker threads\n",
         iWorkerCount);

  m_bShutdown.store(false, std::memory_order_release);
  m_workers.clear();
  m_workers.reserve(iWorkerCount);

  // First create all workers
  for (int i = 0; i < iWorkerCount; ++i) {
    auto pWorker = std::make_unique<TWorkerThread>();
    pWorker->pTaskQueue = std::make_unique<SPSCQueue<TTask>>(QUEUE_SIZE);
    pWorker->uTaskCount.store(0, std::memory_order_relaxed);
    m_workers.push_back(std::move(pWorker));
  }

  // Mark as initialized before starting threads
  m_bInitialized.store(true, std::memory_order_release);

  // Then start threads after all workers are created
  for (int i = 0; i < iWorkerCount; ++i) {
    TWorkerThread *pWorker = m_workers[i].get();
    // Pass worker pointer directly instead of index
    pWorker->thread =
        std::thread(&CGameThreadPool::WorkerThreadProc, this, pWorker);
  }
}

void CGameThreadPool::Destroy() {
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);

  if (!m_bInitialized.load(std::memory_order_acquire))
    return;

  Tracef("CGameThreadPool::Destroy - Shutting down %d worker threads\n",
         GetWorkerCount());

  // Signal shutdown first
  m_bShutdown.store(true, std::memory_order_release);

  // Mark as not initialized to prevent new enqueues
  m_bInitialized.store(false, std::memory_order_release);

  // Join all worker threads
  for (auto &pWorker : m_workers) {
    if (pWorker->thread.joinable())
      pWorker->thread.join();
  }

  m_workers.clear();
}

void CGameThreadPool::WorkerThreadProc(TWorkerThread *pWorker) {
  int iIdleCount = 0;

  while (!m_bShutdown.load(std::memory_order_acquire)) {
    TTask task;

    // Pop from queue with minimal locking
    bool bHasTask = false;
    {
      std::lock_guard<std::mutex> lock(pWorker->queueMutex);
      bHasTask = pWorker->pTaskQueue->Pop(task);
    }

    if (bHasTask) {
      iIdleCount = 0;

      // Execute the task
      try {
        task();
      } catch (const std::exception &e) {
        TraceError("CGameThreadPool::WorkerThreadProc - Exception: %s",
                   e.what());
      } catch (...) {
        TraceError("CGameThreadPool::WorkerThreadProc - Unknown exception");
      }

      pWorker->uTaskCount.fetch_sub(1, std::memory_order_relaxed);
    } else {
      // No work available - idle strategy
      ++iIdleCount;

      if (iIdleCount < 100) {
        // Spin briefly for immediate work
        std::this_thread::yield();
      } else if (iIdleCount < 1000) {
        // Short sleep for moderate idle
        std::this_thread::sleep_for(std::chrono::microseconds(10));
      } else {
        // Longer sleep for extended idle
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // Reset idle count to prevent overflow
        if (iIdleCount > 10000)
          iIdleCount = 1000;
      }
    }
  }

  // Process remaining tasks before shutdown
  TTask task;
  while (true) {
    bool bHasTask = false;
    {
      std::lock_guard<std::mutex> lock(pWorker->queueMutex);
      bHasTask = pWorker->pTaskQueue->Pop(task);
    }

    if (!bHasTask)
      break;

    try {
      task();
    } catch (const std::exception &e) {
      TraceError(
          "CGameThreadPool::WorkerThreadProc - Exception during shutdown: %s",
          e.what());
    } catch (...) {
      TraceError("CGameThreadPool::WorkerThreadProc - Unknown exception during "
                 "shutdown");
    }

    pWorker->uTaskCount.fetch_sub(1, std::memory_order_relaxed);
  }
}

int CGameThreadPool::SelectLeastBusyWorker() const {
  if (m_workers.empty())
    return 0;

  // Simple load balancing: find worker with least pending tasks
  int iBestWorker = 0;
  uint32_t uMinTasks = m_workers[0]->uTaskCount.load(std::memory_order_relaxed);

  for (size_t i = 1; i < m_workers.size(); ++i) {
    uint32_t uTasks = m_workers[i]->uTaskCount.load(std::memory_order_relaxed);
    if (uTasks < uMinTasks) {
      uMinTasks = uTasks;
      iBestWorker = static_cast<int>(i);
    }
  }

  return iBestWorker;
}

size_t CGameThreadPool::GetPendingTaskCount() const {
  size_t uTotal = 0;
  for (const auto &pWorker : m_workers) {
    uTotal += pWorker->uTaskCount.load(std::memory_order_relaxed);
  }
  return uTotal;
}
