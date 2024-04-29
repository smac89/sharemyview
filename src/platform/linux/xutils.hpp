#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xproto.h>

namespace smv::utils {
  struct res
  {
    inline static std::unique_ptr<xcb_connection_t, decltype(&xcb_disconnect)>
      connection { nullptr, &xcb_disconnect };

    inline static std::unique_ptr<xcb_ewmh_connection_t,
                                  decltype(&xcb_ewmh_connection_wipe)>
      ewm_connection { nullptr, &xcb_ewmh_connection_wipe };
  };

  struct xscreen
  {
    xcb_visualid_t               visual;
    decltype(xcb_depth_t::depth) root_depth;
    xcb_image_order_t            order;
    xcb_window_t                 root;
  };
} // namespace smv::utils
