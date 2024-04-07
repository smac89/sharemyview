#pragma once
#include <memory>

#include <spdlog/spdlog.h>
#include <xcb/xcb.h>

namespace smv::common
{
  extern std::shared_ptr<spdlog::logger> logger;
  extern std::unique_ptr<xcb_connection_t, decltype(&xcb_disconnect)>
    connection;
} // namespace smv::common
