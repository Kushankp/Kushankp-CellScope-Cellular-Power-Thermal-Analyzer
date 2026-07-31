#include "cellscope/utils/thread_pool.hpp"

#include <algorithm>

namespace cellscope::utils {

ThreadPool::ThreadPool(std::size_t thread_count) {
  if (thread_count == 0) {
    thread_count = std::max<std::size_t>(1, std::thread::hardware_concurrency());
  }
  workers_.reserve(thread_count);
  for (std::size_t i = 0; i < thread_count; ++i) {
    workers_.emplace_back([this] { run(); });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard lock(mutex_);
    stopping_ = true;
  }
  cv_.notify_all();
  for (auto& worker : workers_) {
    if (worker.joinable()) worker.join();
  }
}

void ThreadPool::run() {
  for (;;) {
    std::function<void()> task;
    {
      std::unique_lock lock(mutex_);
      cv_.wait(lock, [this] { return stopping_ || !tasks_.empty(); });
      if (stopping_ && tasks_.empty()) return;
      task = std::move(tasks_.front());
      tasks_.pop();
    }
    task();
  }
}

}  // namespace cellscope::utils
