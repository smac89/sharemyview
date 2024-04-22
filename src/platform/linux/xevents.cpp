#include "xevents.hpp"
#include "smv/common/fmt.hpp" // IWYU pragma: keep
#include "smv/log.hpp"
#include "xloop.hpp"
#include "xmonitor.hpp"
#include "xutils.hpp"
#include "xwindow.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <tuple>
#include <type_traits>

#include <assert.hpp>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

namespace smv::details {
  using smv::log::logger;

  using smv::utils::res;
  namespace {
    std::shared_mutex listenerMutx;
    std::unordered_map<EventType, std::vector<std::tuple<uint32_t, EventCB>>>
      subscribers;

  } // namespace
  /**
   * @brief Publishes an event
   *
   * @param data the data to send
   */
  template<EventType E, typename D>
  void publishEvent(D data);

  /**
   * @brief Checks if there are any subscribers to an event
   *
   * @param type the event
   * @return true if there are any subscribers to this event
   */
  auto isEventInteresting(EventType type) -> bool;

  XEvents::XEvents()
  {
    VERIFY((res::connection), "X connection is invalid");
    auto new_screens = findNewScreens(mRoots);
    if (!new_screens.empty()) {
      logger->info("Found {} new screens", mRoots.size());
      prepareScreens(new_screens);
      mRoots.insert(mRoots.end(), new_screens.begin(), new_screens.end());
      std::vector<xcb_window_t> children = queryChildren(new_screens);
      monitorChildren(children);
    }
  }

  auto XEvents::getCurrentWindow() const -> xcb_window_t
  {
    std::lock_guard _(mSyncMut);
    return mCurrentWindow.value_or(XCB_NONE);
  }

  void XEvents::start()
  {
    static auto constexpr pollDelayMs = std::chrono::milliseconds(100);

    std::lock_guard _(eventLoopMut);
    mRunning = true;

    xcb_aux_sync(res::connection.get());
    logger->info("Polling for events...");
    for (auto const &root : mRoots) {
      std::unique_ptr<xcb_query_pointer_reply_t> reply(
        xcb_query_pointer_reply(res::connection.get(),
                                xcb_query_pointer(res::connection.get(), root),
                                nullptr));

      if (reply) {
        XEvents::instance().onMouseEnter(
          reply->child, reply->win_x, reply->win_y);
        break;
      }
      std::this_thread::yield();
    }
    while (mRunning) {
      pollEvents();
      std::this_thread::sleep_for(pollDelayMs);
    }
    xcb_aux_sync(res::connection.get());
    logger->info("Polling for events... done");
  }

  void XEvents::stop()
  {
    if (!mRunning.exchange(false)) {
      logger->info("already stopped");
      return;
    }
    // attempting to grab the lock awaits the event loop
    std::lock_guard _(eventLoopMut);
    {
      // once we're here, the event loop should be stopped
      std::lock_guard _(mSyncMut);
      mTracked.clear();
      mRoots.clear();
      mCurrentWindow = std::nullopt;
    }
  }

  void XEvents::onMouseEnter(xcb_window_t window, uint32_t xpos, uint32_t ypos)
  {
    if (std::lock_guard _(mSyncMut); mCurrentWindow == std::nullopt) {
      mCurrentWindow = window;
      if (isEventInteresting(EventType::MouseEnter)) {
        logger->info("Pointer entered window: {:#x}", window);
        watchWindow(window);
        auto evt = EventDataMouseEnter {
          mTracked[window],
          xpos,
          ypos,
        };
        publishEvent<EventType::MouseEnter>(std::move(evt));
      }
    }
  }

  void XEvents::onMouseLeave(xcb_window_t window, uint32_t xpos, uint32_t ypos)
  {
    if (window == XCB_NONE) {
      return;
    }
    if (getCurrentWindow() == window) {
      mCurrentWindow = std::nullopt;
      if (isEventInteresting(EventType::MouseLeave)) {
        logger->info("Pointer leave window: {:#x}", window);
        std::lock_guard _(mSyncMut);
        watchWindow(window);
        auto evt = EventDataMouseLeave {
          mTracked[window],
          xpos,
          ypos,
        };
        publishEvent<EventType::MouseLeave>(std::move(evt));
      }
    }
  }

  void XEvents::onWindowCreated(xcb_window_t window, xcb_window_t parent)
  {
    std::lock_guard _(mSyncMut);
    if (std::find(mRoots.begin(), mRoots.end(), parent) == mRoots.end()) {
      return;
    }
    if (windowIsNormalType(window)) {
      monitorChildren({ window });
      logger->info("Window created: {:#x}", window);
      // Window is not tracked until user interacts with it
    }
  }

  void XEvents::onWindowDestroyed(xcb_window_t window)
  {
    if (isEventInteresting(EventType::WindowClose)) {
      logger->info("Window removed: {:#x}", window);
      std::lock_guard _(mSyncMut);
      if (isWindowWatched(window)) {
        const auto trackedWindow = mTracked[window];
        auto       evt           = EventDataWindowClose { trackedWindow };
        publishEvent<EventType::WindowClose>(std::move(evt));
      }
    }
    if (unwatchWindow(window)) {
      logger->info("Tracked windows: {}", mTracked);
    }
  }

  void XEvents::onWindowMoved(xcb_window_t window, int32_t xpos, int32_t ypos)
  {
    if (isEventInteresting(EventType::WindowMove)) {
      logger->info("Window moved: {:#x}, {}, {}", window, xpos, ypos);
      std::lock_guard _(mSyncMut);
      watchWindow(window);
      int32_t deltaX = 0;
      int32_t deltaY = 0;

      if (auto const &trackedWindow =
            std::dynamic_pointer_cast<XWindow>(mTracked[window])) {
        deltaX = trackedWindow->position().x - xpos;
        deltaY = trackedWindow->position().y - ypos;
        trackedWindow->move(xpos, ypos);
        auto evt = EventDataWindowMove {
          mTracked[window], xpos, ypos, deltaX, deltaY,
        };
        publishEvent<EventType::WindowMove>(std::move(evt));
      } else {
        logger->error("Window {:#x} is not an XWindow", window);
      }
    }
  }

  void XEvents::onWindowResized(xcb_window_t window,
                                uint32_t     width,
                                uint32_t     height)
  {
    if (isEventInteresting(EventType::WindowResize)) {
      logger->info("Window resized: {:#x}, {}, {}", window, width, height);
      std::lock_guard _(mSyncMut);
      watchWindow(window);
      if (auto const &trackedWindow =
            std::dynamic_pointer_cast<XWindow>(mTracked[window])) {
        trackedWindow->resize(width, height);
        auto evt = EventDataWindowResize {
          mTracked[window],
          width,
          height,
        };
        publishEvent<EventType::WindowResize>(std::move(evt));
      } else {
        logger->error("Window {:#x} is not an XWindow", window);
      }
    }
  }

  void XEvents::onWindowRenamed(xcb_window_t               window,
                                std::optional<std::string> name)
  {
    if (isEventInteresting(EventType::WindowRenamed)) {
      std::lock_guard _(mSyncMut);
      watchWindow(window);
      if (name == std::nullopt) {
        name = getWindowName(window);
      }
      logger->info("Window renamed: {:#x}, '{}'", window, name.value());
      if (auto const &trackedWindow =
            std::dynamic_pointer_cast<XWindow>(mTracked[window])) {
        trackedWindow->setName(name.value());
        auto evt = EventDataWindowRenamed {
          mTracked[window],
          name.value(),
        };
        publishEvent<EventType::WindowRenamed>(std::move(evt));
      } else {
        logger->error("Window {:#x} is not an XWindow", window);
      }
    }
  }

  auto XEvents::isWindowWatched(xcb_window_t window) const -> bool
  {
    std::lock_guard _(mSyncMut);
    return mTracked.find(window) != mTracked.end();
  }

  std::weak_ptr<const Window> XEvents::getWatchWindow(xcb_window_t window) const
  {
    std::lock_guard _(mSyncMut);
    if (isWindowWatched(window)) {
      return mTracked.at(window);
    }
    return std::weak_ptr<Window>();
  }

  void XEvents::watchWindow(xcb_window_t window, bool only)
  {
    std::lock_guard _(mSyncMut);
    if (isWindowWatched(window)) {
      return;
    }
    auto windowInfo = getWindowInfo(window);
    if (std::holds_alternative<std::string>(windowInfo)) {
      logger->warn("Window geometry error: {}",
                   std::get<std::string>(windowInfo));
    } else {
      auto info          = std::get<XWindowInfo>(std::move(windowInfo));
      auto trackedWindow = std::make_shared<details::XWindow>(
        window, info.pos.x, info.pos.y, info.size.w, info.size.h);

      if (!info.children.empty()) {
        monitorChildren(info.children);
        trackedWindow->mChildren = std::move(info.children);
      }

      if (!info.name.empty()) {
        trackedWindow->setName(info.name);
      }

      if (info.parent.has_value()) {
        trackedWindow->setParent(&info.parent.value());
        if (auto trackedParent = mTracked.find(info.parent.value());
            trackedParent != mTracked.end()) {
          std::dynamic_pointer_cast<XWindow>(trackedParent->second)
            ->addChild(window);
        }
      } else {
        trackedWindow->setParent(nullptr);
      }
      if (only) {
        // TODO: Do we need this?
        mTracked.clear();
      }
      mTracked[window] = trackedWindow;
    }
    logger->info("Tracked windows: {}", mTracked);
  }

  bool XEvents::unwatchWindow(xcb_window_t window)
  {
    std::lock_guard _(mSyncMut);
    return mTracked.erase(window) > 0;
  }

  void XEvents::unwatchAllWindows()
  {
    std::lock_guard _(mSyncMut);
    mTracked.clear();
  }

  XEvents &XEvents::instance()
  {
    static XEvents instance;
    return instance;
  }

  auto registerEvent(EventType type, EventCB callback) -> Cancel
  {
    /// Registering an event returns a function which can be
    /// called to unsubscribe from the event.
    static std::atomic_uint32_t idPool { 0 };
    std::lock_guard             _ { listenerMutx };

    logger->debug("Subscribing to event: {}", type);

    auto cancelId = ++idPool;
    auto cancelCb = [type, cancelId]() {
      std::lock_guard _ { listenerMutx };
      logger->info("Callback cancelled: {}", type);
      // if the below call throws an exception, it means that
      // there is a problem with the callback itself. i.e. the
      // callback has been called more than once.
      auto &eventSubs = subscribers.at(type);
      eventSubs.erase(std::remove_if(eventSubs.begin(),
                                     eventSubs.end(),
                                     [cancelId](auto const &cancel) {
        return std::get<0>(cancel) == cancelId;
      }),
                      eventSubs.end());

      if (eventSubs.empty()) {
        logger->info("Unsubscribing from event: {}", type);
        subscribers.erase(type);
      }
    };
    auto &eventSubs = subscribers[type];
    eventSubs.emplace_back(cancelId, std::move(callback));
    logger->debug("Subscribed to event: {}", type);
    return [cancelCb = std::move(cancelCb)] {
      static std::once_flag flag;
      // ensure that the callback is only called once
      std::call_once(flag, cancelCb);
    };
  }

  template<EventType E, typename D>
  void publishEvent(D data)
  {
    static_assert(std::is_base_of_v<EventData, D> && D::type == E,
                  "Event type mismatch");

    auto cbs = std::vector<EventCB> {};
    if (std::shared_lock _ { listenerMutx };
        subscribers.find(E) != subscribers.end()) {
      const auto &notifications = subscribers.at(E);
      cbs.reserve(notifications.size());
      std::transform(notifications.begin(),
                     notifications.end(),
                     std::back_inserter(cbs),
                     [](auto const &callback) {
        return std::get<1>(callback);
      });
    } else {
      return;
    }
    logger->debug("Before publishing {} event: {}", E, data);
    std::thread([cbs = std::move(cbs), data = std::move(data)]() {
      for (auto const &callback : cbs) {
        callback(data);
      }
    }).detach();

    logger->debug("After publishing {} event", E);
    std::this_thread::yield();
  }

  auto isEventInteresting(EventType type) -> bool
  {
    std::shared_lock _ { listenerMutx };
    return subscribers.count(type) > 0;
  }
} // namespace smv::details
