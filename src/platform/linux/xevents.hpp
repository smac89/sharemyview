#pragma once

#include "smv/events.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

// clang-format off
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/bundled/color.h>
// clang-format on
#include <xcb/xcb.h>

using TrackedWindows =
  std::unordered_map<xcb_window_t, std::shared_ptr<smv::Window>>;

namespace smv::details {
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
    auto getCurrentWindow() const -> xcb_window_t;

  public:
    explicit XEvents();
    //  force singleton
    XEvents(const XEvents &)                     = delete;
    auto operator=(const XEvents &) -> XEvents & = delete;
    XEvents(XEvents &&)                          = delete;
    auto operator=(XEvents &&) -> XEvents      & = delete;
    ~XEvents()                                   = default;

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
     * @param xpos The mouse x position (relative to the window)
     * @param ypos The mouse y position (relative to the window)
     * @return void
     */
    void onMouseEnter(xcb_window_t window, uint32_t xpos, uint32_t ypos);

    /**
     * @brief a window has been exited by the user
     *
     * @details when the mouse leaves a window, we are notified, and we unset
     * the current window
     * @param window The window that has been exited
     * @param xpos The mouse x position (relative to the window)
     * @param ypos The mouse y position (relative to the window)
     * @return void
     */
    void onMouseLeave(xcb_window_t window, uint32_t xpos, uint32_t ypos);

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
     * @param xpos The new x position
     * @param ypos The new y position
     */
    void onWindowMoved(xcb_window_t window, int32_t xpos, int32_t ypos);

    /**
     * @brief a window has been renamed
     *
     * @param window The window that has been renamed
     * @param name The new name (if any)
     */
    void onWindowRenamed(xcb_window_t               window,
                         std::optional<std::string> name = std::nullopt);

    /**
     * @brief returns if the given window is watched
     *
     * @param window The window to check
     * @return bool
     */
    auto isWindowWatched(xcb_window_t window) const -> bool;

    /**
     * @brief returns the watched window
     *
     * @param window The window to check
     * @return std::weak_ptr<const smv::Window>
     */
    auto getWatchWindow(xcb_window_t window) const
      -> std::weak_ptr<const smv::Window>;

    /**
     * @brief watches the given window for changes
     *
     * @details adds the given window to the list of monitored windows
     * (if not already watched).
     * The changes can be subscribed to by the user
     * @param window The window to track
     * @param only If true, only the window will be watched
     */
    void watchWindow(xcb_window_t window, bool only = false);

    /**
     * @brief unwatch the given window
     *
     * @param window The window to unwatch
     * @return true if the window was being watched and now isn't
     */
    auto unwatchWindow(xcb_window_t window) -> bool;

    /**
     * @brief returns the instance
     *
     * @details Creates a single instance of WindowsManager and returns it
     * @return XEventsMonitor&
     */
    static auto instance() -> XEvents &;

  private:
    /**
     * @brief unwatch all windows
     *
     * @return void
     */
    void unwatchAllWindows();

    std::atomic_bool mRunning = false;
    // marked "mutable" because it's used in some const functions
    mutable std::recursive_mutex mSyncMut {};
    std::vector<xcb_window_t>    mRoots {};
    std::optional<xcb_window_t>  mCurrentWindow {};
    TrackedWindows               mTracked {};
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
   * @param callback The function to call when the event occurs
   * @return Cancel A function which can be used to signal lack of interest in
   * the event
   */
  auto registerEvent(EventType type, EventCB callback) -> Cancel;

  /**
   * @brief Request that an event be triggered
   *
   * @details See https://xcb.freedesktop.org/windowcontextandmanipulation/
   * @param window The window to send the event to
   * @param data The data containing what should be done
   * @param callback The callback to call when the event occurs
   */
  template<EventType E>
  auto requestEvent(xcb_window_t     window,
                    const EventData &data,
                    EventCB          callback) -> std::optional<Cancel>;
} // namespace smv::details

namespace fmt {
  /* https://fmt.dev/9.0.0/api.html#formatting-user-defined-types */
  template<>
  struct formatter<TrackedWindows::value_type>: formatter<std::string>
  {
    template<typename FormatContext>
    auto format(const TrackedWindows::value_type &tracked,
                FormatContext &ctx) const -> decltype(ctx.out())
    {
      return format_to(ctx.out(),
                       R"({:#x}: "{}")",
                       tracked.first,
                       styled(tracked.second->name(), fg(color::yellow_green)));
    }
  };
} // namespace fmt
