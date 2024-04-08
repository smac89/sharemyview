#pragma once

#include "smv/window.hpp"

#include <string_view>
#include <variant>

#include <xcb/xcb.h>

namespace smv::details
{
  struct XGeom
  {
    smv::Position pos;
    smv::Size     size;
  };

  /**
   * @brief broadcast events from the X server and send to the XEvents
   */
  void pollEvents();

  /**
   * determines if the given window is a normal window
   * @param w The window
   * @return true if the window is a normal window
   */
  bool windowIsNormalType(xcb_window_t w);

  /**
   * @brief Get the Window geometry, consisting of the position and size
   *
   * @param w the window id to get the geometry for
   * @return std::variant<XGeom, std::string_view> geom or error string
   */
  std::variant<XGeom, std::string_view> getWindowGeometry(xcb_window_t w);
} // namespace smv::details
