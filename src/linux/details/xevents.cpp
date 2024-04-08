#include "xevents.hpp"
#include "xloop.hpp"
#include "xmonitor.hpp"
#include "xutils.hpp"

#include <chrono>
#include <mutex>
#include <thread>

#include <assert.hpp>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

namespace smv::details
{
  using smv::utils::res;

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
    // xcb_get_geometry_reply_t r;

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
        XEvents::getInstance().onWindowEntered(reply->child);
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

  void XEvents::onWindowEntered(xcb_window_t window)
  {
    std::lock_guard _(mSyncMut);
    if (mCurrentWindow == std::nullopt)
    {
      res::logger->info("Pointer entered window: {}", window);
      mCurrentWindow = window;
      // TODO: track this window
      mTracked[window] = smv::utils::createWindow(window, 0, 0, 0, 0);
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
    }
  }

  void XEvents::onWindowDestroyed(xcb_window_t window)
  {
    std::lock_guard _(mSyncMut);
    res::logger->info("Window removed: {}", window);
  }

  XEvents &XEvents::getInstance()
  {
    static XEvents instance;
    return instance;
  }
} // namespace smv::details
