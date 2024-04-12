#pragma once

#include <vector>
#include <xcb/xcb.h>

namespace smv::details {
  /**
   * @brief Initialize the monitor
   *
   * @return true if the monitor was successfully initialized
   * @return false otherwise
   */
  bool initMonitor();

  /**
   * @brief Deinitialize the monitor
   *
   * @return void
   */
  void deinitMonitor();

  /**
   * Finds new screens that have been created since the last call
   * @param existing_screens the existing screens
   * @return the new screens
   */
  std::vector<xcb_window_t> findNewScreens(
    const std::vector<xcb_window_t> &existing_screens = {});

  /**
   * Registers some events that we are interested in for the given screens
   * @param screens the screens
   */
  void prepareScreens(const std::vector<xcb_window_t> &screens);

  /**
   * @brief For each of the given root windows, find its direct children
   *
   * @param roots the list of root windows
   * @return the list of children
   */
  [[maybe_unused]] std::vector<xcb_window_t> queryChildren(
    const std::vector<xcb_window_t> &roots);

  /**
   * @brief Standalone function to monitor the children of a window
   *
   * @param children
   */
  void monitorChildren(const std::vector<xcb_window_t> &children);
} // namespace smv::details
