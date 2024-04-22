#pragma once

#include <memory>

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

namespace smv::utils {
  struct res
  {
    inline static std::unique_ptr<xcb_connection_t, decltype(&xcb_disconnect)>
      connection { nullptr, &xcb_disconnect };

    inline static std::unique_ptr<xcb_ewmh_connection_t,
                                  decltype(&xcb_ewmh_connection_wipe)>
      ewm_connection { nullptr, &xcb_ewmh_connection_wipe };
  };
} // namespace smv::utils
