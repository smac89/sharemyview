#pragma once

#include "smv/window.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#include <fmt/ranges.h>
#include <xcb/xcb.h>

using TrackedWindows = std::unordered_map<xcb_window_t, smv::Window *>;

namespace smv::details
{
  /**
   * @brief an abstraction of the X Client to handle only certain events
   */
  class XEventsMonitor
  {
    /**
     * @brief returns the current window
     *
     * @return std::optional<xcb_window_t>
     * @details returns the current window
     */
    xcb_window_t getCurrentWindow() const;

  public:
    explicit XEventsMonitor();
    //  force singleton
    XEventsMonitor(const XEventsMonitor &)            = delete;
    XEventsMonitor &operator=(const XEventsMonitor &) = delete;

    /**
     * @brief starts the event loop
     *
     * @return void
     * @details starts the event loop
     */
    void start();

    /**
     * @brief stops the event loop
     *
     * @return void
     * @details flags the event loop to stop and waits for it to finish
     */
    void stop();

    /**
     * @brief a new window has been entered by the user
     *
     * @param window The new current window
     * @return void
     * @details sets the current window if it part of our monitored windows
     */
    void onWindowEntered(xcb_window_t window);

    /**
     * @brief a window has been exited by the user
     *
     * @param window The window that has been exited
     * @return void
     * @details when the mouse leaves a window, we are notified, and we unset
     * the current window
     */
    void onWindowLeave(xcb_window_t window);

    /**
     * @brief new window created event
     *
     * @param window The new window
     * @param parent The parent window
     * @return void
     * @details creates a new window
     */
    void onWindowCreated(xcb_window_t window, xcb_window_t parent);

    /**
     * @brief a window has been destroyed
     *
     * @param window The window that has been destroyed
     * @details when the mouse leaves a window, we are notified, and we unset
     * the current window
     */
    void onWindowDestroyed(xcb_window_t window);

    /**
     * @brief returns the instance
     *
     * @details Creates a single instance of WindowsManager and returns it
     * @return XEventsMonitor&
     */
    static XEventsMonitor &getInstance();

  private:
    std::atomic_bool            mRunning = false;
    std::recursive_mutex        mSyncMut {};
    std::vector<xcb_window_t>   mRoots {};
    std::optional<xcb_window_t> mCurrentWindow {};
    TrackedWindows              mTracked {};
    /**
     * @details inline static means that this declaration of the member will
     * serve as initialization as well
     */
    inline static std::mutex eventLoopMut;
  };
} // namespace smv::details

/* https://github.com/gabime/spdlog?tab=readme-ov-file#user-defined-types */
template<>
struct [[maybe_unused]] fmt::formatter<TrackedWindows>
  : fmt::formatter<std::string>
{
  [[maybe_unused]] auto format(const TrackedWindows &m,
                               format_context &ctx) const -> decltype(ctx.out())
  {
    auto out = fmt::format_to(ctx.out(), "\n{{\n");
    for (const auto &[a, b] : m)
    {
      fmt::format_to(
        out, "  {},\n", fmt::join({ fmt::to_string(a), b->name() }, " : "));
    }
    return fmt::format_to(out, "}}\n");
  }
};
