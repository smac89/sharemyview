#include "smv/utils.hpp"
#include "smv/winclient.hpp"
#include "xevents.hpp"
#include "xhelpers.hpp"
#include "xmonitor.hpp"
#include "xvars.hpp"
#include "xwindow.hpp"

#include <iterator>
#include <memory>
#include <mutex>
#include <utility>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <xcb/xcb.h>

namespace smv
{
  static std::mutex connMutex;

  using smv::common::connection;
  using smv::common::logger;

  void init() noexcept
  {
    std::lock_guard<std::mutex> lk(connMutex);
    if (connection)
    {
      logger->info("X11 connection already established");
      return;
    }
    // initialize the connection to X
    connection.reset(xcb_connect(nullptr, nullptr));

    auto error = xcb_connection_has_error(connection.get());
    if (error)
    {
      logger->error("xcb_connect failed. Error code: {}", error);
      std::ignore = connection.release();
      return;
    }

    if (!smv::details::initHelpers())
    {
      return;
    }

    if (!smv::details::initMonitor())
    {
      return;
    }
    logger->info("X11 connection established");
  }

  void deinit() noexcept
  {
    std::lock_guard<std::mutex> lk(connMutex);
    if (!connection)
    {
      return;
    }
    smv::details::deinitMonitor();
    smv::details::deinitHelpers();
    std::ignore = connection.release();
    logger->info("X11 connection closed");
  }

  void listen(EventType, const EventCallback &cb) {}
  void listen(EventType, const std::uint32_t wid, const EventCallback &cb) {}
} // namespace smv

namespace smv::common
{
  std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt(LOGGER_NAME);

  std::unique_ptr<xcb_connection_t, decltype(&xcb_disconnect)> connection(
    nullptr,
    &xcb_disconnect);
} // namespace smv::common
