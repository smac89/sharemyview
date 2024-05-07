#include "xevents_pub.hpp"

#include <atomic>
#include <chrono>
#include <deque>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;

namespace smv::events::details {
  std::deque<std::function<void()>> notificationQueue;
  std::mutex                        queueMutex;
  std::atomic_bool                  running { false };

  void publishNotifications()
  {
    auto now = std::chrono::steady_clock::now();
    for (auto published = false; running; published = false) {
      std::lock_guard _ { queueMutex };
      for (; !notificationQueue.empty(); published = true) {
        std::thread(notificationQueue.front()).detach();
        notificationQueue.pop_front();
      }
      std::this_thread::yield();
      if (published) {
        // reset the idle timer here
        now = std::chrono::steady_clock::now();
      }
      auto elapsed = std::chrono::steady_clock::now() - now;
      running      = elapsed < maxIdle;
    }
  }
} // namespace smv::events::details
