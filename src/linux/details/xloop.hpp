#pragma once

#include "smv/window.hpp"

#include <string>
#include <variant>

#include <xcb/xcb.h>

namespace smv::details
{
  struct XGeom
  {
    smv::Position pos;
    smv::Size     size;
  };

  struct XWindowInfo
  {
    std::string   name;
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
   * @brief Get the Window Geometry
   *
   * @details The window geometry consists of the position and size
   *
   * @param w the window id to get the geometry for
   * @return std::variant<XGeom, std::string>
   */
  std::variant<XGeom, std::string> getWindowGeometry(xcb_window_t w);

  /**
   * @brief Get the Window Info
   *
   * @details The window info consists of the name, position and size
   *
   * @param w the window id to get the info for
   * @return std::variant<XWindowInfo, std::string>
   */
  std::variant<XWindowInfo, std::string> getWindowInfo(xcb_window_t w);

  /**
   * @brief Get the Window Name
   *
   * @details The window name is the name of the window
   *
   * @param w the window id to get the name for
   * @return std::string
   */
  std::string getWindowName(xcb_window_t w);
} // namespace smv::details
