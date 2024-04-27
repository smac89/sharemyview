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
    xcb_visual_class_t     visual_class;
    std::array<uint8_t, 3> rgb_mask;
    xcb_image_order_t      order;
    uint8_t                root_depth;
    xcb_window_t           root;
  };
} // namespace smv::utils
