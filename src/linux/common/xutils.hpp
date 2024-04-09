#pragma once
#include "smv/winclient.hpp"

#include <memory>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

namespace smv::utils
{
  struct res
  {
    inline static const std::shared_ptr<spdlog::logger> logger =
      spdlog::stderr_color_mt(LOGGER_NAME);

    inline static std::unique_ptr<xcb_connection_t, decltype(&xcb_disconnect)>
      connection { nullptr, &xcb_disconnect };

    inline static std::unique_ptr<xcb_ewmh_connection_t,
                                  decltype(&xcb_ewmh_connection_wipe)>
      ewm_connection { nullptr, &xcb_ewmh_connection_wipe };
  };
} // namespace smv::utils
