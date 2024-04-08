#include "xevents.hpp"
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

namespace smv::details
{
  using smv::utils::res;
  static std::mutex listenerMutx;
  static std::unordered_map<
    EventType,
    std::vector<std::tuple<const uint32_t, const EventCB>>>
    subscribers;

  static void publishEvent(EventType type, const EventData &data);
  static void trackWindow(xcb_window_t);

  XEvents::XEvents()
  {
    VERIFY((res::connection), "X connection is invalid");
    auto new_screens = findNewScreens(mRoots);
    if (!new_screens.empty())
    {
      res::logger->info("Found {} new screens", mRoots.size());
      prepareScreens(new_screens);
      mRoots.insert(mRoots.end(), new_screens.begin(), new_screens.end());
      monitorRoots(new_screens);
    }
  }

  xcb_window_t XEvents::getCurrentWindow() const
  {
    return mCurrentWindow.value_or(XCB_NONE);
  }

  void XEvents::start()
  {
    std::lock_guard<std::mutex> lk(eventLoopMut);
    mRunning = true;

    xcb_aux_sync(res::connection.get());
    res::logger->info("Polling for events...");
    for (auto const &root : mRoots)
    {
      std::unique_ptr<xcb_query_pointer_reply_t> reply(
        xcb_query_pointer_reply(res::connection.get(),
                                xcb_query_pointer(res::connection.get(), root),
                                nullptr));

      if (reply)
      {
        XEvents::getInstance().onWindowEntered(
          reply->child, reply->win_x, reply->win_y);
        break;
      }
      std::this_thread::yield();
    }
    while (mRunning)
    {
      pollEvents();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    xcb_aux_sync(res::connection.get());
    res::logger->info("Polling for events... done");
  }

  void XEvents::stop()
  {
    if (!mRunning.exchange(false))
    {
      res::logger->info("already stopped");
      return;
    }
    std::lock_guard<std::mutex> lk(eventLoopMut);
  }

  void XEvents::onWindowEntered(xcb_window_t window, uint32_t x, uint32_t y)
  {
    std::lock_guard _(mSyncMut);
    if (mCurrentWindow == std::nullopt)
    {
      res::logger->info("Pointer entered window: {}", window);
      mCurrentWindow = window;
      watchWindow(window);
      auto evt =
        EventDataMouseEnter { .x = x, .y = y, { .window = mTracked[window] } };
      publishEvent(EventType::MouseEnter, std::move(evt));
    }
  }

  void XEvents::onWindowLeave(xcb_window_t window)
  {
    std::lock_guard _(mSyncMut);
    if (window == XCB_NONE)
    {
      return;
    }
    if (getCurrentWindow() == window)
    {
      res::logger->info("Pointer leave window: {}", window);
      mCurrentWindow = std::nullopt;
      watchWindow(window);
      auto evt = EventDataMouseLeave { { .window = mTracked[window] } };
      publishEvent(EventType::MouseLeave, std::move(evt));
    }
  }

  void XEvents::onWindowCreated(xcb_window_t window, xcb_window_t parent)
  {
    std::lock_guard _(mSyncMut);
    if (std::find(mRoots.begin(), mRoots.end(), parent) == mRoots.end())
    {
      return;
    }
    if (windowIsNormalType(window))
    {
      monitorChildren({ window });
      res::logger->info("Window created: {}", window);
      // Window is not tracked until user interacts with it
    }
  }

  void XEvents::onWindowDestroyed(xcb_window_t window)
  {
    std::lock_guard _(mSyncMut);
    res::logger->info("Window removed: {}", window);
    if (mTracked.find(window) != mTracked.end())
    {
      const auto trackedWindow = mTracked[window];
      auto       evt = EventDataWindowClose { { .window = trackedWindow } };
      publishEvent(EventType::WindowClose, std::move(evt));
    }
    mTracked.erase(window);
  }

  void XEvents::onWindowMoved(xcb_window_t window, uint32_t x, uint32_t y)
  {
    std::lock_guard _(mSyncMut);
    if (mTracked.find(window) != mTracked.end())
    {
      auto const &trackedWindow =
        std::static_pointer_cast<XWindow>(mTracked[window]);
      int16_t deltaX = trackedWindow->position().x - x,
              deltaY = trackedWindow->position().y - y;
      trackedWindow->move(x, y);

      auto evt = EventDataMove { .x       = x,
                                 .y       = y,
                                 .delta_x = deltaX,
                                 .delta_y = deltaY,
                                 { .window = mTracked[window] } };
      publishEvent(EventType::Move, std::move(evt));
    }
  }

  void XEvents::onWindowResized(xcb_window_t window, uint32_t w, uint32_t h)
  {
    std::lock_guard _(mSyncMut);
    if (mTracked.find(window) != mTracked.end())
    {
      auto const &trackedWindow =
        std::static_pointer_cast<XWindow>(mTracked[window]);
      trackedWindow->resize(w, h);
      auto evt =
        EventDataResize { .w = w, .h = h, { .window = mTracked[window] } };
      publishEvent(EventType::Resize, std::move(evt));
    }
  }

  void XEvents::watchWindow(xcb_window_t window)
  {
    if (mTracked.find(window) == mTracked.end())
    {
      auto windowGeom = getWindowGeometry(window);
      if (std::holds_alternative<std::string_view>(windowGeom))
      {
        res::logger->warn("Window geometry error: {}",
                          std::get<std::string_view>(windowGeom));
      }
      else
      {
        auto const geom  = std::move(std::get<XGeom>(windowGeom));
        mTracked[window] = std::make_shared<details::XWindow>(
          window, geom.pos.x, geom.pos.y, geom.size.w, geom.size.h);
      }
    }
  }

  XEvents &XEvents::getInstance()
  {
    static XEvents instance;
    return instance;
  }

  Cancel registerEvent(EventType type, EventCB cb)
  {
    static std::atomic_uint32_t idPool { 0 };
    std::lock_guard             _ { listenerMutx };

    auto id       = ++idPool;
    auto cancelCb = [type, id]()
    {
      auto &eventSubs = subscribers[type];
      std::remove_if(eventSubs.begin(),
                     eventSubs.end(),
                     [id](auto &cancel) { return std::get<0>(cancel) == id; });
    };
    auto &eventSubs = subscribers[type];
    eventSubs.emplace_back(id, std::move(cb));
    return cancelCb;
  }

  void publishEvent(EventType type, const EventData &data)
  {
    std::lock_guard _ { listenerMutx };
    if (subscribers.find(type) != subscribers.end())
    {
      for (const auto &[_, cb] : subscribers[type])
      {
        cb(data);
      }
    }
  }
} // namespace smv::details
