#pragma once

#include "smv/events.hpp"

#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace smv::events {
  namespace details {
    using namespace std::chrono_literals;
    static auto constexpr maxIdle = 10s;

    extern std::deque<std::function<void()>> notificationQueue;
    extern std::mutex                        queueMutex;
    extern std::atomic_bool                  running;

    void publishNotifications();
  } // namespace details

  template<typename D>
  void enqueueNotifications(
    const D                                             &data,
    std::vector<std::function<void(const EventData &)>> &funcs)
  {

    {
      std::lock_guard _ { details::queueMutex };
      for (auto const &func : funcs) {
        details::notificationQueue.push_back([func, data = data]() {
          func(data);
        });
      }

      if (!details::running.exchange(true)) {
        std::thread(details::publishNotifications).detach();
      }
    }
  }
} // namespace smv::events
