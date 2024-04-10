#include "xevents.hpp"
#include "smv/utils.hpp"
#include "xloop.hpp"
#include "xmonitor.hpp"
#include "xutils.hpp"
#include "xwindow.hpp"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>
#include <tuple>

#include <assert.hpp>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

namespace smv::details {
  using smv::utils::res;
  static std::mutex listenerMutx;
  static std::unordered_map<EventType,
                            std::vector<std::tuple<uint32_t, EventCB>>>
    subscribers;

  /**
   * @brief Publishes an event
   *
   * @param type the event
   * @param data the data to send
   */
  static void publishEvent(EventType type, const EventData &data);

  /**
   * @brief Checks if there are any subscribers to an event
   *
   * @param type the event
   * @return true if there are any subscribers to this event
   */
  static bool isEventInteresting(EventType type);

  XEvents::XEvents()
  {
    VERIFY((res::connection), "X connection is invalid");
    auto new_screens = findNewScreens(mRoots);
    if (!new_screens.empty()) {
      res::logger->info("Found {} new screens", mRoots.size());
      prepareScreens(new_screens);
      mRoots.insert(mRoots.end(), new_screens.begin(), new_screens.end());
      monitorRoots(new_screens);
    }
  }

  xcb_window_t XEvents::getCurrentWindow() const
  {
    std::lock_guard _(mSyncMut);
    return mCurrentWindow.value_or(XCB_NONE);
  }

  void XEvents::start()
  {
    std::lock_guard<std::mutex> lk(eventLoopMut);
    mRunning = true;

    xcb_aux_sync(res::connection.get());
    res::logger->info("Polling for events...");
    for (auto const &root : mRoots) {
      std::unique_ptr<xcb_query_pointer_reply_t> reply(
        xcb_query_pointer_reply(res::connection.get(),
                                xcb_query_pointer(res::connection.get(), root),
                                nullptr));

      if (reply) {
        XEvents::getInstance().onMouseEnter(
          reply->child, reply->win_x, reply->win_y);
        break;
      }
      std::this_thread::yield();
    }
    while (mRunning) {
      pollEvents();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    xcb_aux_sync(res::connection.get());
    res::logger->info("Polling for events... done");
  }

  void XEvents::stop()
  {
    if (!mRunning.exchange(false)) {
      res::logger->info("already stopped");
      return;
    }
    // attempting to grab the lock awaits the event loop
    std::lock_guard<std::mutex> lk(eventLoopMut);
    // once we're here, the event loop should be stopped
    std::lock_guard _(mSyncMut);
    mTracked.clear();
    mRoots.clear();
    mCurrentWindow = std::nullopt;
  }

  void XEvents::onMouseEnter(xcb_window_t window, uint32_t x, uint32_t y)
  {
    std::lock_guard _(mSyncMut);
    if (mCurrentWindow == std::nullopt) {
      mCurrentWindow = window;
      if (isEventInteresting(EventType::MouseEnter)) {
        res::logger->info("Pointer entered window: {}", window);
        watchWindow(window);
        auto evt = EventDataMouseEnter {
          mTracked[window],
          x,
          y,
        };
        publishEvent(EventType::MouseEnter, std::move(evt));
      }
    }
  }

  void XEvents::onMouseLeave(xcb_window_t window, uint32_t x, uint32_t y)
  {
    std::lock_guard _(mSyncMut);
    if (window == XCB_NONE) {
      return;
    }
    if (getCurrentWindow() == window) {
      mCurrentWindow = std::nullopt;
      if (isEventInteresting(EventType::MouseLeave)) {
        res::logger->info("Pointer leave window: {}", window);
        watchWindow(window);
        auto evt = EventDataMouseLeave {
          mTracked[window],
          x,
          y,
        };
        publishEvent(EventType::MouseLeave, std::move(evt));
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
      res::logger->info("Window created: {}", window);
      // Window is not tracked until user interacts with it
    }
  }

  void XEvents::onWindowDestroyed(xcb_window_t window)
  {
    std::lock_guard _(mSyncMut);
    if (isEventInteresting(EventType::WindowClose)) {
      res::logger->info("Window removed: {}", window);
      if (isWindowWatched(window)) {
        const auto trackedWindow = mTracked[window];
        auto       evt           = EventDataWindowClose { trackedWindow };
        publishEvent(EventType::WindowClose, std::move(evt));
      }
      unwatchWindow(window);
    }
  }

  void XEvents::onWindowMoved(xcb_window_t window, uint32_t x, uint32_t y)
  {
    std::lock_guard _(mSyncMut);
    if (isEventInteresting(EventType::Move)) {
      res::logger->info("Window moved: {}, {}, {}", window, x, y);
      watchWindow(window);
      int16_t deltaX = 0, deltaY = 0;

      if (auto const &trackedWindow =
            std::dynamic_pointer_cast<XWindow>(mTracked[window])) {
        deltaX = trackedWindow->position().x - x;
        deltaY = trackedWindow->position().y - y;
        trackedWindow->move(x, y);
        auto evt = EventDataMove {
          mTracked[window], x, y, deltaX, deltaY,
        };
        publishEvent(EventType::Move, std::move(evt));
      } else {
        res::logger->error("Window {} is not an XWindow", window);
      }
    }
  }

  void XEvents::onWindowResized(xcb_window_t window, uint32_t w, uint32_t h)
  {
    std::lock_guard _(mSyncMut);
    if (isEventInteresting(EventType::Resize)) {
      res::logger->info("Window resized: {}, {}, {}", window, w, h);
      watchWindow(window);
      if (auto const &trackedWindow =
            std::dynamic_pointer_cast<XWindow>(mTracked[window])) {
        trackedWindow->resize(w, h);
        auto evt = EventDataResize {
          mTracked[window],
          w,
          h,
        };
        publishEvent(EventType::Resize, std::move(evt));
      } else {
        res::logger->error("Window {} is not an XWindow", window);
      }
    }
  }

  void XEvents::onWindowRenamed(xcb_window_t               window,
                                std::optional<std::string> name)
  {
    std::lock_guard _(mSyncMut);
    if (isEventInteresting(EventType::WindowRenamed)) {
      watchWindow(window);
      if (name == std::nullopt) {
        name = getWindowName(window);
      }
      res::logger->info("Window renamed: {}, '{}'", window, name.value());
      if (auto const &trackedWindow =
            std::dynamic_pointer_cast<XWindow>(mTracked[window])) {
        trackedWindow->setName(name.value());
        auto evt = EventDataWindowRenamed {
          mTracked[window],
          name.value(),
        };
        publishEvent(EventType::WindowRenamed, std::move(evt));
      } else {
        res::logger->error("Window {} is not an XWindow", window);
      }
    }
  }

  bool XEvents::isWindowWatched(xcb_window_t window) const
  {
    std::lock_guard _(mSyncMut);
    return mTracked.find(window) != mTracked.end();
  }

  std::weak_ptr<const Window> XEvents::getWatchedWindow(
    xcb_window_t window) const
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
      res::logger->warn("Window geometry error: {}",
                        std::get<std::string>(windowInfo));
    } else {
      auto info          = std::get<XWindowInfo>(std::move(windowInfo));
      auto trackedWindow = std::make_shared<details::XWindow>(
        window, info.pos.x, info.pos.y, info.size.w, info.size.h);

      if (!info.name.empty()) {
        trackedWindow->setName(info.name);
      }
      if (only) {
        // TODO: Do we need this?
        mTracked.clear();
      }
      mTracked[window] = trackedWindow;
    }
    res::logger->info("Tracked windows: {}", mTracked);
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

  XEvents &XEvents::getInstance()
  {
    static XEvents instance;
    return instance;
  }

  Cancel registerEvent(EventType type, EventCB cb)
  {
    /// Registering an event returns a function which can be
    /// called to unsubscribe from the event.
    /// TODO: How to manually unsubscribe?
    static std::atomic_uint32_t idPool { 0 };
    std::lock_guard             _ { listenerMutx };

    auto id       = ++idPool;
    auto cancelCb = [type, id]() {
      std::lock_guard _ { listenerMutx };
      // if the below call throws an exception, it means that
      // there is a problem with the callback itself. i.e. the
      // callback has been called more than once.
      auto &eventSubs = subscribers.at(type);
      eventSubs.erase(std::remove_if(
        eventSubs.begin(), eventSubs.end(), [id](auto const &cancel) {
        return std::get<0>(cancel) == id;
      }));

      if (eventSubs.empty()) {
        res::logger->info("Unsubscribing from event: {}", type);
        subscribers.erase(type);
      }
    };
    auto &eventSubs = subscribers[type];
    eventSubs.emplace_back(id, std::move(cb));
    return [cancelCb = std::move(cancelCb)] {
      static std::once_flag flag;
      // ensure that the callback is only called once
      std::call_once(flag, cancelCb);
    };
  }

  void publishEvent(EventType type, const EventData &data)
  {
    std::lock_guard _ { listenerMutx };
    if (subscribers.find(type) != subscribers.end()) {
      for (auto const &[_, cb] : subscribers[type]) {
        cb(data);
      }
    }
  }

  bool isEventInteresting(EventType type)
  {
    std::lock_guard _ { listenerMutx };
    return subscribers.count(type) > 0;
  }
} // namespace smv::details
