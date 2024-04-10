#include "smv/events.hpp"
#include "smv/utils.hpp"
#include "smv/winclient.hpp"
#include "xevents.hpp"
#include "xmonitor.hpp"
#include "xtools.hpp"
#include "xutils.hpp"
#include "xwindow.hpp"
#ifdef SMV_TRACK_SUBSCRIPTIONS
#include "smv/autocancel.hpp"
#endif

#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <xcb/xcb.h>

using smv::utils::res;

namespace smv {
  static std::mutex              connMutex, listenGuard;
  static std::condition_variable waitListenCond;

  static bool initConnection();
  static void deinitConnection();

  void init() noexcept
  {
    std::lock_guard lk(connMutex);
    if (res::connection) {
      res::logger->info("X11 connection already established");
      return;
    }

    if (!initConnection()) {
      return;
    }

    if (!details::initTools()) {
      return;
    }

    if (!details::initMonitor()) {
      return;
    }
    res::logger->info("X11 connection established");
    waitListenCond.notify_all();
  }

  void deinit() noexcept
  {
    std::lock_guard lk(connMutex);
    if (!res::connection) {
      return;
    }
    details::deinitMonitor();
    details::deinitTools();
    deinitConnection();
    // TODO: clear all subscriptions
    res::logger->info("X11 connection closed");
  }

  bool initConnection()
  {
    // initialize the connection to X
    res::connection.reset(xcb_connect(nullptr, nullptr));

    auto error = xcb_connection_has_error(res::connection.get());
    if (error) {
      res::logger->error("xcb_connect failed. Error code: {}", error);
      std::ignore = res::connection.release();
    }
    return res::connection != nullptr;
  }

  void deinitConnection()
  {
    std::ignore = res::connection.release();
  }

  void waitConnection()
  {
    std::unique_lock<std::mutex> lk(listenGuard);
    if (!res::connection) {
      waitListenCond.wait(lk, [] {
        return res::connection != nullptr;
      });
    }
  }

  Cancel listen(EventType type, EventCB cb)
  {
    waitConnection();
    std::lock_guard lk(listenGuard);
#ifdef SMV_TRACK_SUBSCRIPTIONS
    return autoCancel(details::registerEvent(type, std::move(cb)));
#else
    return details::registerEvent(type, std::move(cb));
#endif
  }
} // namespace smv
