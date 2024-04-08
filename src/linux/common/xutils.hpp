#pragma once
#include "smv/winclient.hpp"

#include <memory>

#include <spdlog/spdlog.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

namespace smv::utils
{
  struct res
  {
    inline static const std::shared_ptr<spdlog::logger> logger =
      spdlog::stdout_color_mt(LOGGER_NAME);

    inline static std::unique_ptr<xcb_connection_t, decltype(&xcb_disconnect)>
      connection { nullptr, &xcb_disconnect };

    inline static std::unique_ptr<xcb_ewmh_connection_t,
                                  decltype(&xcb_ewmh_connection_wipe)>
      ewm_connection { nullptr, &xcb_ewmh_connection_wipe };
  };

  // extern std::shared_ptr<spdlog::logger> logger;
  // extern std::unique_ptr<xcb_connection_t, decltype(&xcb_disconnect)>
  //                                     connection;
  extern std::shared_ptr<smv::Window> createWindow(const uint32_t id,
                                                   int32_t        x,
                                                   int32_t        y,
                                                   uint32_t       w,
                                                   uint32_t       h);
} // namespace smv::utils
