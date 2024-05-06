#include "xmonitor.hpp"
#include "smv/common/raw_iter.hpp"
#include "smv/log.hpp"
#include "xevents.hpp"
#include "xutils.hpp"

#include <memory>
#include <thread>

#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xproto.h>

namespace smv::details {
  using smv::utils::res, smv::log::logger;

  auto initMonitor() -> bool
  {
    if (res::ewm_connection) {
      return true;
    }
    decltype(res::ewm_connection) ewm(new xcb_ewmh_connection_t,
                                      &xcb_ewmh_connection_wipe);
    if (xcb_ewmh_init_atoms_replies(
          ewm.get(),
          xcb_ewmh_init_atoms(res::connection.get(), ewm.get()),
          nullptr)) {
      res::ewm_connection.swap(ewm);
      logger->info("Connected to compatible window manager!");

      std::thread(&XEvents::start, &XEvents::instance()).detach();
    } else {
      logger->info("Not connected to compatible window manager!");
    }
    return res::ewm_connection != nullptr;
  }

  void deinitMonitor()
  {
    XEvents::instance().stop();
    res::ewm_connection.reset();
  }

  auto findNewScreens(const std::vector<xcb_window_t> &existing_screens)
    -> std::vector<xcb_window_t>
  {
    std::vector<xcb_window_t> new_screens;
    if (!res::connection) {
      return new_screens;
    }

    const auto *const setup   = xcb_get_setup(res::connection.get());
    auto              screens = xcb_setup_roots_iterator(setup);
    new_screens.reserve(setup->roots_len);
    //  most x11 servers will have only one screen/root window, but it's
    //  possible to have more
    logger->info("Found {} screens", setup->roots_len);

    for (; screens.rem > 0; xcb_screen_next(&screens)) {
      auto root = screens.data->root;
      logger->debug("Screen {} with root {}",
                    screens.index / xcb_screen_sizeof(screens.data),
                    root);
      if (std::find(existing_screens.begin(), existing_screens.end(), root) ==
          existing_screens.end()) {
        new_screens.push_back(root);
      }
    }

    const int num_screens = static_cast<int>(new_screens.size());

    for (int i = 0; i < num_screens; i++) {
      xcb_ewmh_get_windows_reply_t reply {};
      if (xcb_ewmh_get_virtual_roots_reply(
            res::ewm_connection.get(),
            xcb_ewmh_get_virtual_roots_unchecked(res::ewm_connection.get(), i),
            &reply,
            nullptr)) {

        std::unique_ptr<xcb_ewmh_get_windows_reply_t,
                        decltype(&xcb_ewmh_get_windows_reply_wipe)>
          _defer(&reply, &xcb_ewmh_get_windows_reply_wipe);

        logger->debug("Screen {} with {} virtual roots", i, reply.windows_len);
        for (const auto &win :
             utils::RawIterator(reply.windows, reply.windows_len)) {
          if (std::find(existing_screens.begin(),
                        existing_screens.end(),
                        win) == existing_screens.end()) {
            new_screens.push_back(win);
          }
        }
      }
    }
    return new_screens;
  }

  void prepareScreens(const std::vector<xcb_window_t> &screens)
  {
    // initialization is done this way to avoid compiler warnings about
    // unintialized fields
    static constexpr auto root_mask = []() constexpr {
      xcb_change_window_attributes_value_list_t mask {};
      mask.event_mask = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
      // XCB_EVENT_MASK_STRUCTURE_NOTIFY |
      // XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW,
      return mask;
    }();

    for (auto screen : screens) {
      // for each root window, register that we want to be notified of
      // the given events
      std::ignore = xcb_change_window_attributes_aux(
        res::connection.get(), screen, XCB_CW_EVENT_MASK, &root_mask);
      /* std::ignore = xcb_grab_pointer(connection.get(), 1, w,
                                      XCB_EVENT_MASK_POINTER_MOTION,
                                      XCB_GRAB_MODE_ASYNC,
                                      XCB_GRAB_MODE_ASYNC, XCB_NONE,
                                      XCB_NONE, XCB_CURRENT_TIME);
      */
    }
  }

  auto queryChildren(const std::vector<xcb_window_t> &roots, bool recursive)
    -> std::vector<xcb_window_t>
  {
    std::vector<std::pair<xcb_window_t, xcb_query_tree_cookie_t>> ch_query;

    for (auto root : roots) {
      logger->debug("Querying children of {:#x}", root);
      ch_query.emplace_back(
        root, xcb_query_tree_unchecked(res::connection.get(), root));
    }
    std::vector<xcb_window_t> all_children;

    for (auto const &[parent, cookie] : ch_query) {
      auto query_tree_rep = std::unique_ptr<xcb_query_tree_reply_t>(
        xcb_query_tree_reply(res::connection.get(), cookie, nullptr));

      logger->debug(
        "{:#x} has {} children", parent, query_tree_rep->children_len);
      auto *children = xcb_query_tree_children(query_tree_rep.get());
      if (children == nullptr || query_tree_rep->children_len == 0) {
        logger->debug("no children for {:#x}", parent);
        continue;
      }

      logger->debug("Children of {:#x}: [{:#x}]",
                    parent,
                    fmt::join(smv::utils::RawIterator(
                                children, query_tree_rep->children_len),
                              ", "));

      all_children.reserve(all_children.size() + query_tree_rep->children_len);
      for (const auto &child :
           smv::utils::RawIterator(children, query_tree_rep->children_len)) {
        all_children.push_back(child);
      }
    }

    if (recursive && !all_children.empty()) {
      auto descendants = queryChildren(all_children, recursive);
      all_children.reserve(all_children.size() + descendants.size());
      all_children.insert(
        all_children.end(), descendants.begin(), descendants.end());
    }
    return all_children;
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

    // intialization is done this way to avoid compiler warnings about
    // unintialized fields
    static constexpr auto root_mask = []() constexpr {
      xcb_change_window_attributes_value_list_t mask {};
      mask.event_mask =
        XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
        XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
      return mask;
    }();

    for (auto child : children) {
      //    xcb_input_xi_select_events(connection.get(), w, 1,
      //    &child_mask.mask);
      std::ignore = xcb_change_window_attributes_aux(
        res::connection.get(), child, XCB_CW_EVENT_MASK, &root_mask);
    }
  }
} // namespace smv::details
