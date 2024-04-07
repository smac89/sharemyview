#include "xevents.hpp"
#include "xmonitor.hpp"
#include "xvars.hpp"

#include <chrono>
#include <mutex>
#include <thread>

#include <assert.hpp>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

namespace smv::details
{
  using smv::common::connection;
  using smv::common::logger;

  XEventsMonitor::XEventsMonitor()
  {
    VERIFY((connection), "X connection is invalid");
    auto new_screens = findNewScreens(mRoots);
    if (!new_screens.empty())
    {
      logger->info("Found {} new screens", mRoots.size());
      prepareScreens(new_screens);
      mRoots.insert(mRoots.end(), new_screens.begin(), new_screens.end());
      monitorRoots(new_screens);
    }
  }

  xcb_window_t XEventsMonitor::getCurrentWindow() const
  {
    return mCurrentWindow.value_or(XCB_NONE);
  }

  void XEventsMonitor::start()
  {
    std::lock_guard<std::mutex> lk(eventLoopMut);
    mRunning = true;

    xcb_aux_sync(connection.get());
    logger->info("Polling for events...");
    for (auto const &root : mRoots)
    {
      std::unique_ptr<xcb_query_pointer_reply_t> reply(xcb_query_pointer_reply(
        connection.get(), xcb_query_pointer(connection.get(), root), nullptr));

      if (reply)
      {
        XEventsMonitor::getInstance().onWindowEntered(reply->child);
        break;
      }
      std::this_thread::yield();
    }
    while (mRunning)
    {
      pollEvents();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    xcb_aux_sync(connection.get());
    logger->info("Polling for events... done");
  }

  void XEventsMonitor::stop()
  {
    if (!mRunning.exchange(false))
    {
      logger->info("already stopped");
      return;
    }
    std::lock_guard<std::mutex> lk(eventLoopMut);
  }

  void XEventsMonitor::onWindowEntered(xcb_window_t window)
  {
    std::lock_guard _(mSyncMut);
    if (mCurrentWindow == std::nullopt)
    {
      logger->info("Pointer entered window: {}", window);
      mCurrentWindow = window;
    }
  }

  void XEventsMonitor::onWindowLeave(xcb_window_t window)
  {
    std::lock_guard _(mSyncMut);
    if (window == XCB_NONE)
    {
      return;
    }
    if (getCurrentWindow() == window)
    {
      logger->info("Pointer leave window: {}", window);
      mCurrentWindow = std::nullopt;
    }
  }

  void XEventsMonitor::onWindowCreated(xcb_window_t window, xcb_window_t parent)
  {
    std::lock_guard _(mSyncMut);
    if (std::find(mRoots.begin(), mRoots.end(), parent) == mRoots.end())
    {
      return;
    }
    if (windowIsNormalType(window))
    {
      monitorChildren({ window });
      logger->info("Window created: {}", window);
    }
  }

  void XEventsMonitor::onWindowDestroyed(xcb_window_t window)
  {
    std::lock_guard _(mSyncMut);
    logger->info("Window removed: {}", window);
  }

  XEventsMonitor &XEventsMonitor::getInstance()
  {
    static XEventsMonitor instance;
    return instance;
  }
} // namespace smv::details
