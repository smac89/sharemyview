#include "xmonitor.hpp"
#include "smv/utils.hpp"
#include "xevents.hpp"
#include "xtools.hpp"
#include "xutils.hpp"

#include <thread>

#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>
#include <xcb/xcb_ewmh.h>

static std::unique_ptr<xcb_ewmh_connection_t,
                       decltype(&xcb_ewmh_connection_wipe)>
  ewm_connection(nullptr, &xcb_ewmh_connection_wipe);

namespace smv::details
{
  using smv::utils::res;

  bool initMonitor()
  {
    if (ewm_connection)
    {
      return true;
    }
    auto *ewm = new xcb_ewmh_connection_t;
    if (xcb_ewmh_init_atoms_replies(
          ewm, xcb_ewmh_init_atoms(res::connection.get(), ewm), nullptr))
    {
      ewm_connection.reset(ewm);
      res::logger->info("Connected to compatible window manager!");

      std::thread(&XEvents::start, &XEvents::getInstance()).detach();
    }
    else
    {
      res::logger->info("Not connected to compatible window manager!");
    }
    return ewm_connection != nullptr;
  }

  void deinitMonitor()
  {
    XEvents::getInstance().stop();
    ewm_connection.reset();
  }

  std::vector<xcb_window_t> findNewScreens(
    const std::vector<xcb_window_t> &existing_screens)
  {
    std::vector<xcb_window_t> new_screens;
    if (!res::connection)
    {
      return new_screens;
    }

    auto setup   = xcb_get_setup(res::connection.get());
    auto screens = xcb_setup_roots_iterator(setup);
    new_screens.reserve(setup->roots_len);
    //  most x11 servers will have only one screen
    res::logger->info("Found {} screens", setup->roots_len);

    for (; screens.rem > 0; xcb_screen_next(&screens))
    {
      auto root = screens.data->root;
      res::logger->debug("Screen {} with root {}",
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

        res::logger->debug(
          "Screen {} with {} virtual roots", i, reply.windows_len);
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
        res::connection.get(), w, XCB_CW_EVENT_MASK, &root_mask);
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
      res::logger->debug("Querying children of {}", w);
      ch_query.emplace_back(w,
                            xcb_query_tree_unchecked(res::connection.get(), w));
    }

    std::vector<xcb_window_t> all_children;

    for (auto const &[parent, cookie] : ch_query)
    {
      auto query_tree_rep = std::unique_ptr<xcb_query_tree_reply_t>(
        xcb_query_tree_reply(res::connection.get(), cookie, nullptr));

      res::logger->debug(
        "{} has {} children", parent, query_tree_rep->children_len);
      auto children = xcb_query_tree_children(query_tree_rep.get());
      if (children == nullptr || query_tree_rep->children_len == 0)
      {
        res::logger->warn("no children for {}", parent);
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
                    XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY,
    };

    for (auto w : children)
    {
      //    xcb_input_xi_select_events(connection.get(), w, 1,
      //    &child_mask.mask);
      std::ignore = xcb_change_window_attributes_aux(
        res::connection.get(), w, XCB_CW_EVENT_MASK, &root_mask);
    }
  }
} // namespace smv::details
