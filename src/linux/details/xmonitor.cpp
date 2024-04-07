#include "xmonitor.hpp"
#include "smv/utils.hpp"
#include "xevents.hpp"
#include "xhelpers.hpp"
#include "xvars.hpp"

#include <thread>

#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>
#include <xcb/xcb_ewmh.h>

static std::unique_ptr<xcb_ewmh_connection_t,
                       decltype(&xcb_ewmh_connection_wipe)>
  ewm_connection(nullptr, &xcb_ewmh_connection_wipe);

namespace smv::details
{
  using smv::common::connection;
  using smv::common::logger;

  bool initMonitor()
  {
    if (ewm_connection)
    {
      return true;
    }
    auto *ewm = new xcb_ewmh_connection_t;
    if (xcb_ewmh_init_atoms_replies(
          ewm, xcb_ewmh_init_atoms(connection.get(), ewm), nullptr))
    {
      ewm_connection.reset(ewm);
      logger->info("Connected to compatible window manager!");

      std::thread(&XEventsMonitor::start, &XEventsMonitor::getInstance())
        .detach();
    }
    else
    {
      logger->info("Not connected to compatible window manager!");
    }
    return ewm_connection != nullptr;
  }

  void deinitMonitor()
  {
    XEventsMonitor::getInstance().stop();
    ewm_connection.reset();
  }

  std::vector<xcb_window_t> findNewScreens(
    const std::vector<xcb_window_t> &existing_screens)
  {
    std::vector<xcb_window_t> new_screens;
    if (!connection)
    {
      return new_screens;
    }

    auto setup   = xcb_get_setup(connection.get());
    auto screens = xcb_setup_roots_iterator(setup);
    new_screens.reserve(setup->roots_len);
    //  most x11 servers will have only one screen
    logger->info("Found {} screens", setup->roots_len);

    for (; screens.rem > 0; xcb_screen_next(&screens))
    {
      auto root = screens.data->root;
      logger->debug("Screen {} with root {}",
                    screens.index / xcb_screen_sizeof(screens.data),
                    root);
      if (std::find(existing_screens.begin(), existing_screens.end(), root) ==
          existing_screens.end())
      {
        new_screens.push_back(root);
      }
    }

    const int num_screens = static_cast<int>(new_screens.size());

    for (int i = 0; i < num_screens; i++)
    {
      xcb_ewmh_get_windows_reply_t reply {};
      if (xcb_ewmh_get_virtual_roots_reply(
            ewm_connection.get(),
            xcb_ewmh_get_virtual_roots_unchecked(ewm_connection.get(), i),
            &reply,
            nullptr))
      {

        std::unique_ptr<xcb_ewmh_get_windows_reply_t,
                        decltype(&xcb_ewmh_get_windows_reply_wipe)>
          _defer(&reply, &xcb_ewmh_get_windows_reply_wipe);

        logger->debug("Screen {} with {} virtual roots", i, reply.windows_len);
        std::copy_if(reply.windows,
                     reply.windows + reply.windows_len,
                     std::back_inserter(new_screens),
                     [&existing_screens](auto w)
                     {
                       return std::find(existing_screens.begin(),
                                        existing_screens.end(),
                                        w) == existing_screens.end();
                     });
      }
    }
    return new_screens;
  }

  void prepareScreens(const std::vector<xcb_window_t> &screens)
  {
    static constexpr xcb_change_window_attributes_value_list_t root_mask {
      .event_mask = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
      // XCB_EVENT_MASK_STRUCTURE_NOTIFY |
      // XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW,
    };

    for (auto w : screens)
    {
      // for each root window, register that we want to be notified of
      // the given events
      std::ignore = xcb_change_window_attributes_aux(
        connection.get(), w, XCB_CW_EVENT_MASK, &root_mask);
      /* std::ignore = xcb_grab_pointer(connection.get(), 1, w,
                                      XCB_EVENT_MASK_POINTER_MOTION,
                                      XCB_GRAB_MODE_ASYNC,
                                      XCB_GRAB_MODE_ASYNC, XCB_NONE,
                                      XCB_NONE, XCB_CURRENT_TIME);
      */
    }
  }

  void monitorRoots(const std::vector<xcb_window_t> &roots)
  {
    std::vector<std::pair<xcb_window_t, xcb_query_tree_cookie_t>> ch_query;

    for (auto w : roots)
    {
      logger->debug("Querying children of {}", w);
      ch_query.emplace_back(w, xcb_query_tree_unchecked(connection.get(), w));
    }

    std::vector<xcb_window_t> all_children;

    for (auto const &[parent, cookie] : ch_query)
    {
      auto query_tree_rep = std::unique_ptr<xcb_query_tree_reply_t>(
        xcb_query_tree_reply(connection.get(), cookie, nullptr));

      logger->debug("{} has {} children", parent, query_tree_rep->children_len);
      auto children = xcb_query_tree_children(query_tree_rep.get());
      if (children == nullptr || query_tree_rep->children_len == 0)
      {
        logger->warn("no children for {}", parent);
        continue;
      }

      all_children.reserve(all_children.size() + query_tree_rep->children_len);
      for (const auto &c :
           smv::utils::CPtrIterator(children, query_tree_rep->children_len))
      {
        all_children.push_back(c);
      }
    }
    monitorChildren(all_children);
  }

  void monitorChildren(const std::vector<xcb_window_t> &children)
  {
    //  static constexpr struct {
    //    xcb_input_event_mask_t mask = {.deviceid =
    //    XCB_INPUT_DEVICE_ALL_MASTER,
    //                                   .mask_len = 1};
    //    xcb_input_xi_event_mask_t xiem =
    //    static_cast<xcb_input_xi_event_mask_t>(
    //        XCB_INPUT_XI_EVENT_MASK_ENTER | XCB_INPUT_XI_EVENT_MASK_LEAVE);
    //  } child_mask;

    static constexpr xcb_change_window_attributes_value_list_t root_mask {
      .event_mask = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
                    XCB_EVENT_MASK_EXPOSURE,
    };

    for (auto w : children)
    {
      //    xcb_input_xi_select_events(connection.get(), w, 1,
      //    &child_mask.mask);
      std::ignore = xcb_change_window_attributes_aux(
        connection.get(), w, XCB_CW_EVENT_MASK, &root_mask);
    }
  }

  bool windowIsNormalType(const xcb_window_t w)
  {

    auto window_type_req =
      xcb_ewmh_get_wm_window_type_unchecked(ewm_connection.get(), w);
    xcb_ewmh_get_atoms_reply_t atom_reply;

    if (!xcb_ewmh_get_wm_window_type_reply(
          ewm_connection.get(), window_type_req, &atom_reply, nullptr))
    {
      return false;
    }

    //  created as a way to free the reply once the function returns
    std::unique_ptr<xcb_ewmh_get_atoms_reply_t,
                    decltype(&xcb_ewmh_get_atoms_reply_wipe)>
      deferred(&atom_reply, &xcb_ewmh_get_atoms_reply_wipe);

    return atom_reply.atoms_len > 0 &&
           *atom_reply.atoms == ewm_connection->_NET_WM_WINDOW_TYPE_NORMAL;
  }

  void pollEvents()
  {
    std::shared_ptr<xcb_generic_event_t> event;
    for (event.reset(xcb_poll_for_event(connection.get())); event != nullptr;
         event.reset(xcb_poll_for_event(connection.get())))
    {

      if (event->response_type == XCB_NONE)
      {
        auto err = std::reinterpret_pointer_cast<xcb_generic_error_t>(event);
        if (err->error_code != XCB_WINDOW)
        {
          // XCB_WINDOW is the error code for an invalid window
          logger->error("Received error: {}", getErrorString(err->error_code));
        }
      }
      // See https://www.x.org/wiki/Development/Documentation/XGE/
      else if (event->response_type == XCB_GE_GENERIC)
      {
        auto gevent =
          std::reinterpret_pointer_cast<xcb_ge_generic_event_t>(event);
        logger->info("Received generic event {}", getEventName(event));
      }
      else
      {
        switch (event->response_type & ~0x80)
        {
            /*
             * NOTE: we only care about the fact that a window is entered
             * or left. Whether or not it used to be inferior to another window
             * is irrelevant.
             *
             * See:
             * https://www.x.org/releases/current/doc/xproto/x11protocol.html#events:pointer_window
             * */
          case XCB_ENTER_NOTIFY:
          {
            auto enter =
              std::reinterpret_pointer_cast<xcb_enter_notify_event_t>(event);
            if (enter->detail != XCB_NOTIFY_DETAIL_INFERIOR)
            {
              XEventsMonitor::getInstance().onWindowEntered(enter->event);
            }
            break;
          }
          case XCB_LEAVE_NOTIFY:
          {
            auto leave =
              std::reinterpret_pointer_cast<xcb_leave_notify_event_t>(event);
            if (leave->detail != XCB_NOTIFY_DETAIL_INFERIOR)
            {
              XEventsMonitor::getInstance().onWindowLeave(leave->event);
            }
            break;
          }
          case XCB_CREATE_NOTIFY:
          {
            auto create =
              std::reinterpret_pointer_cast<xcb_create_notify_event_t>(event);
            XEventsMonitor::getInstance().onWindowCreated(create->window,
                                                          create->parent);
            break;
          }
          case XCB_DESTROY_NOTIFY:
          {
            auto destroy =
              std::reinterpret_pointer_cast<xcb_destroy_notify_event_t>(event);
            XEventsMonitor::getInstance().onWindowDestroyed(destroy->window);
            break;
          }
          default:
          {
            logger->info("Received default event: {}", getEventName(event));
            break;
          }
        }
      }
    }
  }
} // namespace smv::details
