#pragma once

#include <condition_variable>
#include <cstddef>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace cellscope::utils {

class ThreadPool {
 public:
  explicit ThreadPool(std::size_t thread_count = 0);
  ~ThreadPool();

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  template <class Fn>
  auto submit(Fn&& fn) -> std::future<std::invoke_result_t<Fn>> {
    using Result = std::invoke_result_t<Fn>;
    auto task = std::make_shared<std::packaged_task<Result()>>(std::forward<Fn>(fn));
    auto future = task->get_future();
    {
      std::lock_guard lock(mutex_);
      tasks_.push([task] { (*task)(); });
    }
    cv_.notify_one();
    return future;
  }

  std::size_t size() const noexcept { return workers_.size(); }

 private:
  void run();

  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  bool stopping_{false};
};

}  // namespace cellscope::utils
