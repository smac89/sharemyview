#pragma once

#include "smv/events.hpp"

#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <type_traits>
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

  /**
   * @brief queues up a notification for delivery
   * @details ensures the notifications are delivered in the order they were
   * received
   * @tparam D the type of the notification
   * @param data
   * @param funcs
   */
  template<typename D,
           typename = std::enable_if<std::is_base_of_v<EventData, D>>>
  void enqueueNotifications(
    const D                                             &data,
    std::vector<std::function<void(const EventData &)>> &funcs)
  {
    {
      std::lock_guard _ { details::queueMutex };
      for (auto &func : funcs) {
        details::notificationQueue.emplace_back(
          [func = std::move(func), data = data]() {
          func(data);
        });
      }

      if (!details::running.exchange(true)) {
        std::thread(details::publishNotifications).detach();
      }
    }
  }
} // namespace smv::events
