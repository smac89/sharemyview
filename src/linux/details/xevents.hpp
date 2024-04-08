#pragma once

#include "smv/winclient.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#include <fmt/ranges.h>
#include <xcb/xcb.h>

using TrackedWindows =
  std::unordered_map<xcb_window_t, std::shared_ptr<smv::Window>>;

namespace smv::details
{
  /**
   * @brief an abstraction of the X Client to handle only certain events
   */
  class XEvents
  {
    /**
     * @brief returns the current window
     *
     * @return std::optional<xcb_window_t>
     */
    xcb_window_t getCurrentWindow() const;

  public:
    explicit XEvents();
    //  force singleton
    XEvents(const XEvents &)            = delete;
    XEvents &operator=(const XEvents &) = delete;

    /**
     * @brief starts the event loop
     *
     * @return void
     */
    void start();

    /**
     * @brief stops the event loop
     *
     * @details flags the event loop to stop and waits for it to finish
     * @return void
     */
    void stop();

    /**
     * @brief a new window has been entered by the user
     *
     * @details sets the current window if it part of our monitored windows
     * @param window The new current window
     * @param x The mouse x position (relative to the window)
     * @param y The mouse y position (relative to the window)
     * @return void
     */
    void onWindowEntered(xcb_window_t window, uint32_t x, uint32_t y);

    /**
     * @brief a window has been exited by the user
     *
     * @details when the mouse leaves a window, we are notified, and we unset
     * the current window
     * @param window The window that has been exited
     * @return void
     */
    void onWindowLeave(xcb_window_t window);

    /**
     * @brief new window created event
     *
     * @details creates a new window
     * @param window The new window
     * @param parent The parent window
     * @return void
     */
    void onWindowCreated(xcb_window_t window, xcb_window_t parent);

    /**
     * @brief a window has been destroyed
     *
     * @details when the mouse leaves a window, we are notified, and we unset
     * the current window
     * @param window The window that has been destroyed
     */
    void onWindowDestroyed(xcb_window_t window);

    /**
     * @brief a window has been resized
     *
     * @param window The window that has been resized
     * @param width The new width
     * @param height The new height
     */
    void onWindowResized(xcb_window_t window, uint32_t width, uint32_t height);

    /**
     * @brief a window has been moved
     *
     * @param window The window that has been moved
     * @param x The new x position
     * @param y The new y position
     */
    void onWindowMoved(xcb_window_t window, uint32_t x, uint32_t y);

    /**
     * @brief watches the given window for changes
     *
     * @details adds the given window to the list of monitored windows.
     * The changes can be subscribed to by the user
     * @param window The window to track
     */
    void watchWindow(xcb_window_t window);

    /**
     * @brief returns the instance
     *
     * @details Creates a single instance of WindowsManager and returns it
     * @return XEventsMonitor&
     */
    static XEvents &getInstance();

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

  /**
   * @brief Register to listen for events
   *
   * @param type The type of event to listen for
   * @param cb The function to call when the event occurs
   * @return Cancel A function which can be used to signal lack of interest in
   * the event
   */
  Cancel registerEvent(EventType type, EventCB cb);
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
