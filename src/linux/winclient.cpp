#include "winclient.hpp"
#include "smv/utils.hpp"
#include "smv/windowobject.hpp"
#include <iterator>
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <thread>
#include <tuple>
#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>
#include <xcb/xcb_ewmh.h>

namespace smv::impl {
  static std::unique_ptr<xcb_ewmh_connection_t,
                         decltype(&xcb_ewmh_connection_wipe)>
    ewm_connection(nullptr, &xcb_ewmh_connection_wipe);
  static bool init_ewm();
} // namespace smv::impl

namespace smv {
  using namespace impl;
  static std::mutex connMutex;

  void init() noexcept
  {
    std::lock_guard<std::mutex> lk(connMutex);
    if (connection) {
      spdlog::info("X11 connection already established");
      return;
    }
    // initialize the connection to X
    connection.reset(xcb_connect(nullptr, nullptr));

    auto error = xcb_connection_has_error(connection.get());
    if (error) {
      spdlog::error("xcb_connect failed. Error code: {}", error);
      std::ignore = connection.release();
      return;
    }

    if (init_ewm()) {
      spdlog::info("Connected to compatible window manager!");
    } else {
      spdlog::info("Not connected to compatible window manager!");
      return;
    }

    // initialize the error context for xcb errors
    xcb_errors_context_t *ctx = nullptr;
    if (xcb_errors_context_new(connection.get(), &ctx) < 0) {
      spdlog::error("error creating xcb_errors_context");
      return;
    }
    err_ctx.reset(ctx, &xcb_errors_context_free);

    std::thread(&WindowsClient::start, &WindowsClient::getInstance()).detach();
    spdlog::info("X11 connection established");
  }

  void deinit() noexcept
  {
    std::lock_guard<std::mutex> lk(connMutex);
    if (!connection) {
      return;
    }
    WindowsClient::getInstance().stop();
    std::ignore = connection.release();
    spdlog::info("X11 connection closed");
  }

  void listen(const EventType, const EventCallback &) {}
} // namespace smv

namespace smv::impl {
  std::unique_ptr<xcb_connection_t, decltype(&xcb_disconnect)> connection(
    nullptr,
    &xcb_disconnect);
  std::shared_ptr<xcb_errors_context_t> err_ctx;

  bool init_ewm()
  {
    if (ewm_connection) {
      return true;
    }
    auto *ewm = new xcb_ewmh_connection_t;
    if (xcb_ewmh_init_atoms_replies(
          ewm, xcb_ewmh_init_atoms(connection.get(), ewm), nullptr)) {
      ewm_connection.reset(ewm);
      return true;
    }
    return false;
  }

  // create some atoms for querying the server
  [[nodiscard]] std::vector<xcb_window_t> findNewScreens(
    const std::vector<xcb_window_t> &existing_screens)
  {
    std::vector<xcb_window_t> new_screens;
    if (!connection) {
      return new_screens;
    }

    auto setup = xcb_get_setup(connection.get());
    auto screens = xcb_setup_roots_iterator(setup);
    new_screens.reserve(setup->roots_len);
    //  most x11 servers will have only one screen
    spdlog::info("Found {} screens", setup->roots_len);

    for (; screens.rem > 0; xcb_screen_next(&screens)) {
      auto root = screens.data->root;
      spdlog::info("Screen {} with root {}",
                   screens.index / xcb_screen_sizeof(screens.data),
                   root);
      if (std::find(existing_screens.begin(), existing_screens.end(), root) ==
          existing_screens.end()) {
        new_screens.push_back(root);
      }
    }

    const int num_screens = static_cast<int>(new_screens.size());

    for (int i = 0; i < num_screens; i++) {
      xcb_ewmh_get_windows_reply_t reply{};
      if (xcb_ewmh_get_virtual_roots_reply(
            ewm_connection.get(),
            xcb_ewmh_get_virtual_roots_unchecked(ewm_connection.get(), i),
            &reply,
            nullptr)) {

        std::unique_ptr<xcb_ewmh_get_windows_reply_t,
                        decltype(&xcb_ewmh_get_windows_reply_wipe)>
          _defer(&reply, &xcb_ewmh_get_windows_reply_wipe);

        spdlog::info("Screen {} with {} virtual mRoots", i, reply.windows_len);
        std::copy_if(reply.windows,
                     reply.windows + reply.windows_len,
                     std::back_inserter(new_screens),
                     [&existing_screens](auto w) {
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
    static constexpr xcb_change_window_attributes_value_list_t root_mask{
      .event_mask = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                    //                    XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                    XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW,
    };

    for (auto w : screens) {
      // for each root window, register that we want to be notified of
      // the given events
      std::ignore = xcb_change_window_attributes_aux(
        connection.get(), w, XCB_CW_EVENT_MASK, &root_mask);
      /* std::ignore = xcb_grab_pointer(connection.get(), 1, w,
                                      XCB_EVENT_MASK_POINTER_MOTION,
                                      XCB_GRAB_MODE_ASYNC,
                                      XCB_GRAB_MODE_ASYNC, XCB_NONE,
                                      XCB_NONE, XCB_CURRENT_TIME);
       break;*/
    }
  }

  [[nodiscard]] std::vector<xcb_window_t> mapChildren(
    const std::vector<xcb_window_t> &mRoots,
    ChildScreenMapType &window_roots)
  {
    std::vector<std::tuple<xcb_window_t, xcb_query_tree_cookie_t>> ch_query;

    for (auto w : mRoots) {
      spdlog::info("Querying children of {}", w);
      ch_query.emplace_back(w, xcb_query_tree_unchecked(connection.get(), w));
    }

    std::vector<xcb_window_t> all_children;

    for (auto const &[parent, cookie] : ch_query) {
      auto query_tree_rep = std::unique_ptr<xcb_query_tree_reply_t>(
        xcb_query_tree_reply(connection.get(), cookie, nullptr));

      spdlog::info("{} has {} children", parent, query_tree_rep->children_len);
      auto children = xcb_query_tree_children(query_tree_rep.get());
      if (children == nullptr || query_tree_rep->children_len == 0) {
        spdlog::error("no children for {}", parent);
        continue;
      }

      all_children.reserve(all_children.size() + query_tree_rep->children_len);
      window_roots.reserve(window_roots.size() + query_tree_rep->children_len);
      for (const auto &c :
           smv::utils::CPtrIterator(children, query_tree_rep->children_len)) {
        all_children.push_back(c);
        window_roots[c] = parent;
      }
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

    static constexpr xcb_change_window_attributes_value_list_t root_mask{
      .event_mask = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
                    XCB_EVENT_MASK_EXPOSURE,
    };

    for (auto w : children) {
      //    xcb_input_xi_select_events(connection.get(), w, 1,
      //    &child_mask.mask);
      std::ignore = xcb_change_window_attributes_aux(
        connection.get(), w, XCB_CW_EVENT_MASK, &root_mask);
    }
  }

  void pollEvents()
  {
    std::shared_ptr<xcb_generic_event_t> event;
    for (event.reset(xcb_poll_for_event(connection.get())); event != nullptr;
         event.reset(xcb_poll_for_event(connection.get()))) {

      if (event->response_type == XCB_NONE) {
        auto err = std::reinterpret_pointer_cast<xcb_generic_error_t>(event);
        if (err->error_code != XCB_WINDOW) {
          // XCB_WINDOW is the error code for an invalid window
          spdlog::error("Received error: {}", getErrorString(err->error_code));
        }
      }
      // See https://www.x.org/wiki/Development/Documentation/XGE/
      else if (event->response_type == XCB_GE_GENERIC) {
        auto gevent =
          std::reinterpret_pointer_cast<xcb_ge_generic_event_t>(event);
        spdlog::info("Received generic event {}", getEventName(event));
      } else {
        switch (event->response_type & ~0x80) {
            /*
             * NOTE: we only care about the fact that a window is entered
             * or left. Whether or not it used to be inferior to another window
             * is irrelevant.
             *
             * See:
             * https://www.x.org/releases/current/doc/xproto/x11protocol.html#events:pointer_window
             * */
          case XCB_ENTER_NOTIFY: {
            auto enter =
              std::reinterpret_pointer_cast<xcb_enter_notify_event_t>(event);
            if (enter->detail != XCB_NOTIFY_DETAIL_INFERIOR) {
              WindowsClient::getInstance().onWindowEntered(enter->event);
            }
            break;
          }
          case XCB_LEAVE_NOTIFY: {
            auto leave =
              std::reinterpret_pointer_cast<xcb_leave_notify_event_t>(event);
            if (leave->detail != XCB_NOTIFY_DETAIL_INFERIOR) {
              WindowsClient::getInstance().onWindowLeave(leave->event);
            }
            break;
          }
          case XCB_CREATE_NOTIFY: {
            auto create =
              std::reinterpret_pointer_cast<xcb_create_notify_event_t>(event);
            WindowsClient::getInstance().onWindowCreated(create->window,
                                                         create->parent);
            break;
          }
          case XCB_DESTROY_NOTIFY: {
            auto destroy =
              std::reinterpret_pointer_cast<xcb_destroy_notify_event_t>(event);
            WindowsClient::getInstance().onWindowDestroyed(destroy->window);
            break;
          }
          default: {
            spdlog::info("Received default event: {}", getEventName(event));
            break;
          }
        }
      }
    }
  }

  [[maybe_unused]] std::string getErrorString(uint8_t error_code)
  {
    return xcb_errors_get_name_for_error(err_ctx.get(), error_code, nullptr);
  }

  [[maybe_unused]] std::string getEventName(
    const std::weak_ptr<xcb_generic_event_t> &event)
  {
    if (auto locked_sp = event.lock()) {
      return xcb_errors_get_name_for_xcb_event(
        err_ctx.get(), locked_sp.get(), nullptr);
    }
    spdlog::error("The event is already destroyed");
    return "unknown";
  }

  bool windowIsNormalType(const xcb_window_t w)
  {

    auto window_type_req =
      xcb_ewmh_get_wm_window_type_unchecked(ewm_connection.get(), w);
    xcb_ewmh_get_atoms_reply_t atom_reply;

    if (!xcb_ewmh_get_wm_window_type_reply(
          ewm_connection.get(), window_type_req, &atom_reply, nullptr)) {
      return false;
    }

    //  created as a way to free the reply once the function returns
    std::unique_ptr<xcb_ewmh_get_atoms_reply_t,
                    decltype(&xcb_ewmh_get_atoms_reply_wipe)>
      deferred(&atom_reply, &xcb_ewmh_get_atoms_reply_wipe);

    return atom_reply.atoms_len > 0 &&
           *atom_reply.atoms == ewm_connection->_NET_WM_WINDOW_TYPE_NORMAL;
  }
} // namespace smv::impl
