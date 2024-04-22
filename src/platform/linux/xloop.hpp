#pragma once

#include "smv/window.hpp"

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <xcb/xcb.h>

namespace smv::details {
  struct XGeom
  {
    smv::Position pos;
    smv::Size     size;
  };

  struct XWindowInfo
  {
    std::string                 name;
    smv::Position               pos;
    smv::Size                   size;
    std::optional<xcb_window_t> parent;
    std::vector<xcb_window_t>   children;
  };

  /**
   * @brief broadcast events from the X server and send to the XEvents
   */
  void pollEvents();

  /**
   * determines if the given window is a normal window
   * @param window The window
   * @return true if the window is a normal window
   */
  auto windowIsNormalType(xcb_window_t window) -> bool;

  /**
   * @brief Get the Parent of this Window
   *
   * @details The parent is the parent window
   *
   * @param window the window id to get the parent for
   * @return std::optional<xcb_window_t>
   */
  auto getParent(xcb_window_t window) -> std::optional<xcb_window_t>;

  /**
   * @brief Get the Window Geometry
   *
   * @details The window geometry consists of the position and size
   *
   * @param window the window id to get the geometry for
   * @return std::variant<XGeom, std::string>
   */
  auto getWindowGeometry(xcb_window_t window)
    -> std::variant<XGeom, std::string>;

  /**
   * @brief Get the Window Info
   *
   * @details The window info consists of the name, position and size
   *
   * @param window the window id to get the info for
   * @return std::variant<XWindowInfo, std::string>
   */
  auto getWindowInfo(xcb_window_t window)
    -> std::variant<XWindowInfo, std::string>;

  /**
   * @brief Get the Window Name
   *
   * @details The window name is the name of the window
   *
   * @param window the window id to get the name for
   * @return std::string
   */
  auto getWindowName(xcb_window_t window) -> std::string;
} // namespace smv::details
