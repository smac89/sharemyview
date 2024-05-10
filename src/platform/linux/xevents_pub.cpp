#include "xevents_pub.hpp"
#include "smv/log.hpp"

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

  using log::logger;

  void publishNotifications()
  {
    auto now = std::chrono::steady_clock::now();
    for (auto published = false; running; published = false) {
      for (; !notificationQueue.empty(); published = true) {
        notificationQueue
          .front()(); // NOTE: we can only reliably do this because dequeue does
                      // not invalidate elements when resizing
        std::lock_guard _ { queueMutex };
        notificationQueue.pop_front();
      }
      logger->debug("Idling publisher for 4ms...");
      std::this_thread::sleep_for(std::chrono::milliseconds(4)); // no spin
      if (published) {
        // reset the idle timer here
        now = std::chrono::steady_clock::now();
      }
      auto elapsed = std::chrono::steady_clock::now() - now;
      running      = elapsed < maxIdle;
    }
  }
} // namespace smv::events::details
