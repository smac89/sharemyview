#include "smv/events.hpp"
#include "smv/winclient.hpp"
#include "xevents.hpp"
#include "xmonitor.hpp"
#include "xtools.hpp"
#include "xutils.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>

#include <xcb/xcb.h>

namespace smv {
  using smv::utils::res;
  static std::mutex              connMutex, listenGuard;
  static std::condition_variable waitListenCond;

  static auto initConnection() -> bool;
  static void deinitConnection();

  void init() noexcept
  {
    std::lock_guard _(connMutex);
    if (res::connection) {
      spdlog::apply_logger_env_levels(logger);
      logger->info("X11 connection already established");
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
    logger->info("X11 connection established");
    waitListenCond.notify_all();
  }

  void deinit() noexcept
  {
    std::lock_guard _(connMutex);
    if (!res::connection) {
      return;
    }
    details::deinitMonitor();
    details::deinitTools();
    deinitConnection();
    logger->info("X11 connection closed");
  }

  auto initConnection() -> bool
  {
    // initialize the connection to X
    res::connection.reset(xcb_connect(nullptr, nullptr));

    auto error = xcb_connection_has_error(res::connection.get());
    if (error) {
      logger->error("xcb_connect failed. Error code: {}", error);
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
    if (std::unique_lock lock(listenGuard); !res::connection) {
      waitListenCond.wait(lock, [] {
        return res::connection != nullptr;
      });
    }
  }

  auto listen(EventType type, EventCB callback) -> Cancel
  {
    waitConnection();
    return details::registerEvent(type, std::move(callback));
  }
} // namespace smv
