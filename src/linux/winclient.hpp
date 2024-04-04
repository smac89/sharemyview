#pragma once

#include <algorithm>
#include <assert.hpp>
#include <atomic>
#include <cassert>
#include <chrono>
#include <fmt/ranges.h>
#include <iterator>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_errors.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xinput.h>

using ChildScreenMapType = std::unordered_map<xcb_window_t, xcb_window_t>;

namespace smv::impl {
  /**
   * Finds new screens that have been created since the last call
   * @param existing_screens the existing screens
   * @return the new screens
   */
  [[nodiscard]] static std::vector<xcb_window_t> findNewScreens(
    const std::vector<xcb_window_t> &existing_screens);

  /**
   * Registers some events that we are interested in for the given screens
   * @param screens the screens
   */
  static void prepareScreens(const std::vector<xcb_window_t> &screens);

  [[nodiscard]] static std::vector<xcb_window_t> mapChildren(
    const std::vector<xcb_window_t> &mRoots,
    ChildScreenMapType &child_map);

  static void monitorChildren(const std::vector<xcb_window_t> &children);

  /**
   * determines if the given window is a normal window
   * @param w The window
   * @return true if the window is a normal window
   */
  static bool windowIsNormalType(xcb_window_t w);

  static void pollEvents();

  static std::string getErrorString(uint8_t);

  static std::string getEventName(
    const std::weak_ptr<xcb_generic_event_t> &event);

  extern std::unique_ptr<xcb_connection_t, decltype(&xcb_disconnect)>
    connection;
  extern std::shared_ptr<xcb_errors_context_t> err_ctx;

  class WindowsClient
  {
  public:
    explicit WindowsClient()
    {
      VERIFY((connection), "X connection is invalid");
    }

    //  force singleton
    WindowsClient(const WindowsClient &) = delete;
    WindowsClient &operator=(const WindowsClient &) = delete;

    /**
     * @brief starts the event loop
     *
     * @return void
     * @details starts the event loop
     */
    void start()
    {
      VERIFY((connection), "X connection is invalid");
      std::lock_guard<std::mutex> lk(eventLoopMut);
      mRunning = true;
      auto new_screens = findNewScreens(mRoots);
      if (!new_screens.empty()) {
        spdlog::info("Found {} new screens", mRoots.size());
        prepareScreens(new_screens);
        mRoots.insert(mRoots.end(), new_screens.begin(), new_screens.end());
        auto children = mapChildren(new_screens, mWindowScreens);
        monitorChildren(children);
      }

      spdlog::info("Monitoring: {}", mWindowScreens);

      xcb_aux_sync(connection.get());
      spdlog::info("Polling for events...");
      for (auto const &root : mRoots) {
        std::unique_ptr<xcb_query_pointer_reply_t> reply(
          xcb_query_pointer_reply(connection.get(),
                                  xcb_query_pointer(connection.get(), root),
                                  nullptr));

        if (reply) {
          WindowsClient::getInstance().onWindowEntered(reply->child);
          break;
        }
        std::this_thread::yield();
      }
      while (mRunning) {
        pollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      xcb_aux_sync(connection.get());
      spdlog::info("Polling for events... done");
    }

    /**
     * @brief stops the event loop
     *
     * @return void
     * @details flags the event loop to stop and waits for it to finish
     */
    void stop()
    {
      if (!mRunning.exchange(false)) {
        spdlog::info("already stopped");
        return;
      }
      std::lock_guard<std::mutex> lk(eventLoopMut);
    }

    /**
     * @brief returns the current window
     *
     * @return std::optional<xcb_window_t>
     * @details returns the current window
     */
    xcb_window_t getCurrentWindow() const
    {
      return mCurrentWindow.value_or(XCB_NONE);
    }

    /**
     * @brief a new window has been entered by the user
     *
     * @param window The new current window
     * @return void
     * @details sets the current window if it part of our monitored windows
     */
    void onWindowEntered(xcb_window_t window)
    {
      std::lock_guard _(mSyncMut);
      if (mWindowScreens.find(window) != mWindowScreens.end()) {
        if (mCurrentWindow == std::nullopt) {
          spdlog::info("Pointer entered window: {}", window);
          mCurrentWindow = window;
        }
      }
    }

    /**
     * @brief a window has been exited by the user
     *
     * @param window The window that has been exited
     * @return void
     * @details when the mouse leaves a window, we are notified, and we unset
     * the current window
     */
    void onWindowLeave(xcb_window_t window)
    {
      std::lock_guard _(mSyncMut);
      if (window == XCB_NONE) {
        return;
      }
      if (getCurrentWindow() == window) {
        spdlog::info("Pointer leave window: {}", window);
        mCurrentWindow = std::nullopt;
      }
    }

    void onWindowDestroyed(xcb_window_t window)
    {
      std::lock_guard _(mSyncMut);
      if (mWindowScreens.erase(window) > 0) {
        spdlog::info("Window removed: {}", window);
      }
    }

    /**
     * @brief new window created event
     *
     * @param window The new window
     * @param parent The parent window
     * @return void
     * @details creates a new window
     */
    void onWindowCreated(xcb_window_t window, xcb_window_t parent) noexcept
    {
      std::lock_guard _(mSyncMut);
      if (std::find(mRoots.begin(), mRoots.end(), parent) == mRoots.end()) {
        return;
      }
      if (windowIsNormalType(window)) {
        mWindowScreens[window] = parent;
        monitorChildren({ window });
        spdlog::info("Window created: {}", window);
      }
    }

    /**
     * @brief returns the instance
     *
     * @return WindowsManager&
     * @details Creates a single instance of WindowsManager and returns it
     */
    static WindowsClient &getInstance()
    {
      static WindowsClient instance;
      return instance;
    }

  private:
    std::atomic_bool mRunning = false;
    //  Access to resources is guarded by this mutex
    std::recursive_mutex mSyncMut{};
    std::vector<xcb_window_t> mRoots{};
    std::optional<xcb_window_t> mCurrentWindow{};
    ChildScreenMapType mWindowScreens{};
    inline static std::mutex eventLoopMut;
  };
} // namespace smv::impl

/* https://github.com/gabime/spdlog?tab=readme-ov-file#user-defined-types */
template<>
struct [[maybe_unused]] fmt::formatter<ChildScreenMapType>
  : fmt::formatter<std::string>
{
  [[maybe_unused]] auto format(const ChildScreenMapType &m,
                               format_context &ctx) const -> decltype(ctx.out())
  {
    auto out = fmt::format_to(ctx.out(), "\n{{\n");
    for (const auto &[a, b] : m) {
      fmt::format_to(out, "  {},\n", fmt::join({ a, b }, " : "));
    }
    return fmt::format_to(out, "}}\n");
  }
};
