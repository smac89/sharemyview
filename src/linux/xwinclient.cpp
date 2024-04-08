#include "smv/utils.hpp"
#include "smv/winclient.hpp"
#include "xevents.hpp"
#include "xmonitor.hpp"
#include "xtools.hpp"
#include "xutils.hpp"
#include "xwindow.hpp"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <xcb/xcb.h>

using smv::utils::res;

namespace smv
{
  static std::mutex                                               connMutex;
  std::unordered_map<EventType, std::vector<const EventCallback>> subscribers;

  static bool initConnection();
  static void deinitConnection();

  void init() noexcept
  {
    std::lock_guard<std::mutex> lk(connMutex);
    if (res::connection)
    {
      res::logger->info("X11 connection already established");
      return;
    }

    if (!initConnection())
    {
      return;
    }

    if (!smv::details::initTools())
    {
      return;
    }

    if (!smv::details::initMonitor())
    {
      return;
    }
    res::logger->info("X11 connection established");
  }

  void deinit() noexcept
  {
    std::lock_guard<std::mutex> lk(connMutex);
    if (!res::connection)
    {
      return;
    }
    smv::details::deinitMonitor();
    smv::details::deinitTools();
    deinitConnection();
    res::logger->info("X11 connection closed");
  }

  bool initConnection()
  {
    // initialize the connection to X
    res::connection.reset(xcb_connect(nullptr, nullptr));

    auto error = xcb_connection_has_error(res::connection.get());
    if (error)
    {
      res::logger->error("xcb_connect failed. Error code: {}", error);
      std::ignore = res::connection.release();
    }
    return res::connection != nullptr;
  }

  void deinitConnection()
  {
    std::ignore = res::connection.release();
  }

  void listen(EventType type, const EventCallback cb)
  {
    subscribers[type].push_back(std::move(cb));
  }
  void listen(EventType type, const std::uint32_t wid, EventCallback cb) {}
} // namespace smv
