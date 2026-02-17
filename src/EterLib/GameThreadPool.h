#pragma once

#include "SPSCQueue.h"
#include "EterBase/Singleton.h"
#include <thread>
#include <vector>
#include <functional>
#include <future>
#include <atomic>
#include <memory>
#include <mutex>

class CGameThreadPool : public CSingleton<CGameThreadPool> {
public:
  using TTask = std::function<void()>;

  CGameThreadPool();
  ~CGameThreadPool();

  // Initialize thread pool with specified worker count
  // If count <= 0, uses hardware_concurrency
  void Initialize(int iWorkerCount = -1);

  // Shutdown and join all worker threads
  void Destroy();

  // Enqueue a task and get a future to track completion
  template <typename TFunc> std::future<void> Enqueue(TFunc &&func);

  // Get number of active workers
  int GetWorkerCount() const { return static_cast<int>(m_workers.size()); }

  // Get approximate number of pending tasks across all queues
  size_t GetPendingTaskCount() const;

  // Check if pool is initialized
  bool IsInitialized() const {
    return m_bInitialized.load(std::memory_order_acquire);
  }

private:
  struct TWorkerThread {
    std::thread thread;
    std::unique_ptr<SPSCQueue<TTask>> pTaskQueue;
    std::mutex
        queueMutex; // Mutex to protect SPSC queue from multiple producers
    std::atomic<uint32_t> uTaskCount;

    TWorkerThread() : uTaskCount(0) {}
  };

  void WorkerThreadProc(TWorkerThread *pWorker);
  int SelectLeastBusyWorker() const;

  std::vector<std::unique_ptr<TWorkerThread>> m_workers;
  std::atomic<bool> m_bShutdown;
  std::atomic<bool> m_bInitialized;
  mutable std::mutex m_lifecycleMutex; // Protects initialization/destruction

  static const size_t QUEUE_SIZE = 8192;
};

// Template implementation
template <typename TFunc>
std::future<void> CGameThreadPool::Enqueue(TFunc &&func) {
  // Lock to ensure thread pool isn't being destroyed
  std::unique_lock<std::mutex> lock(m_lifecycleMutex);

  if (!m_bInitialized.load(std::memory_order_acquire)) {
    // If not initialized, execute on calling thread
    lock.unlock(); // No need to hold lock

    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();
    try {
      func();
      promise->set_value();
    } catch (...) {
      promise->set_exception(std::current_exception());
    }
    return future;
  }

  // Create a promise to track task completion
  auto promise = std::make_shared<std::promise<void>>();
  auto future = promise->get_future();

  // Wrap function in shared_ptr to avoid move issues with std::function
  auto pFunc = std::make_shared<typename std::decay<TFunc>::type>(
      std::forward<TFunc>(func));

  // Wrap the task with promise completion
  TTask task = [promise, pFunc]() {
    try {
      (*pFunc)();
      promise->set_value();
    } catch (...) {
      promise->set_exception(std::current_exception());
    }
  };

  // Select worker with least load
  int iWorkerIndex = SelectLeastBusyWorker();
  TWorkerThread *pWorker = m_workers[iWorkerIndex].get();

  // Increment task count before pushing
  pWorker->uTaskCount.fetch_add(1, std::memory_order_relaxed);

  // Try to enqueue the task with mutex protection for SPSC queue
  bool bPushed = false;
  {
    std::lock_guard<std::mutex> queueLock(pWorker->queueMutex);
    bPushed = pWorker->pTaskQueue->Push(std::move(task));
  }

  if (!bPushed) {
    // Queue is full, decrement count and execute on calling thread as fallback
    pWorker->uTaskCount.fetch_sub(1, std::memory_order_relaxed);

    // Release lifecycle lock before executing task
    lock.unlock();

    try {
      (*pFunc)();
      promise->set_value();
    } catch (...) {
      promise->set_exception(std::current_exception());
    }
  }

  return future;
}
