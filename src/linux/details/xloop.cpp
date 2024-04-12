#include "xloop.hpp"
#include "xevents.hpp"
#include "xmonitor.hpp"
#include "xtools.hpp"
#include "xutils.hpp"

#include <optional>

#include <xcb/xcb_ewmh.h>

namespace smv::details {
  using smv::utils::res, smv::log::logger;

  void pollEvents()
  {
    std::shared_ptr<xcb_generic_event_t> event;
    auto                                &xevents = XEvents::getInstance();
    for (event.reset(xcb_poll_for_event(res::connection.get()));
         event != nullptr;
         event.reset(xcb_poll_for_event(res::connection.get()))) {

      if (event->response_type == XCB_NONE) {
        auto err = std::reinterpret_pointer_cast<xcb_generic_error_t>(event);
        if (err->error_code != XCB_WINDOW) {
          // XCB_WINDOW is the error code for an invalid window
          logger->error("Received error: {}",
                        getErrorCodeName(err->error_code));
        }
      }
      // See https://www.x.org/wiki/Development/Documentation/XGE/
      else if (event->response_type == XCB_GE_GENERIC) {
        auto gevent =
          std::reinterpret_pointer_cast<xcb_ge_generic_event_t>(event);
        logger->info("Received generic event {}", getEventName(event.get()));
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
              xevents.onMouseEnter(
                enter->event, enter->event_x, enter->event_y);
            }
            break;
          }
          case XCB_LEAVE_NOTIFY: {
            auto leave =
              std::reinterpret_pointer_cast<xcb_leave_notify_event_t>(event);
            if (leave->detail != XCB_NOTIFY_DETAIL_INFERIOR) {
              xevents.onMouseLeave(
                leave->event, leave->event_x, leave->event_y);
            }
            break;
          }
          case XCB_CREATE_NOTIFY: {
            auto create =
              std::reinterpret_pointer_cast<xcb_create_notify_event_t>(event);
            xevents.onWindowCreated(create->window, create->parent);
            break;
          }
          case XCB_DESTROY_NOTIFY: {
            auto destroy =
              std::reinterpret_pointer_cast<xcb_destroy_notify_event_t>(event);
            xevents.onWindowDestroyed(destroy->window);
            break;
          }
          case XCB_CONFIGURE_NOTIFY: {
            auto configure =
              std::reinterpret_pointer_cast<xcb_configure_notify_event_t>(
                event);
            if (auto window =
                  xevents.getWatchedWindow(configure->window).lock()) {
              if (window->position().x != configure->x ||
                  window->position().y != configure->y) {
                xevents.onWindowMoved(
                  configure->window, configure->x, configure->y);
              } else {
                xevents.onWindowResized(
                  configure->window, configure->width, configure->height);
              }
            }
            break;
          }
          case XCB_PROPERTY_NOTIFY: {
            auto prop =
              std::reinterpret_pointer_cast<xcb_property_notify_event_t>(event);
            if (prop->state == XCB_PROPERTY_NEW_VALUE &&
                (prop->atom == res::ewm_connection->_NET_WM_NAME ||
                 prop->atom == XCB_ATOM_WM_NAME)) {
              xevents.onWindowRenamed(prop->window);
            }
            break;
          }
          default: {
            logger->info("Received default event: {}",
                         getEventName(event.get()));
            break;
          }
        }
      }
    }
  }

  bool windowIsNormalType(const xcb_window_t w)
  {

    auto window_type_req =
      xcb_ewmh_get_wm_window_type_unchecked(res::ewm_connection.get(), w);
    xcb_ewmh_get_atoms_reply_t atom_reply;

    if (!xcb_ewmh_get_wm_window_type_reply(
          res::ewm_connection.get(), window_type_req, &atom_reply, nullptr)) {
      return false;
    }

    //  created as a way to free the reply once the function returns
    std::unique_ptr<xcb_ewmh_get_atoms_reply_t,
                    decltype(&xcb_ewmh_get_atoms_reply_wipe)>
      deferred(&atom_reply, &xcb_ewmh_get_atoms_reply_wipe);

    return atom_reply.atoms_len > 0 &&
           *atom_reply.atoms == res::ewm_connection->_NET_WM_WINDOW_TYPE_NORMAL;
  }

  std::optional<xcb_window_t> getParent(xcb_window_t w)
  {
    xcb_query_tree_reply_t *reply =
      xcb_query_tree_reply(res::connection.get(),
                           xcb_query_tree_unchecked(res::connection.get(), w),
                           nullptr);
    if (reply->parent == XCB_NONE) {
      return std::nullopt;
    } else {
      return reply->parent;
    }
  }

  std::variant<XGeom, std::string> getWindowGeometry(xcb_window_t w)
  {
    xcb_generic_error_t                      *err = nullptr;
    std::shared_ptr<xcb_get_geometry_reply_t> reply(xcb_get_geometry_reply(
      res::connection.get(), xcb_get_geometry(res::connection.get(), w), &err));
    std::shared_ptr<xcb_generic_error_t>      _ { err };

    decltype(getWindowGeometry(w)) resp;

    if (err) {
      resp = getErrorCodeName(err->error_code);
    } else {
      resp = XGeom { .pos  = { .x = reply->x, .y = reply->y },
                     .size = { .w = reply->width, .h = reply->height } };
    }
    return resp;
  }

  std::string getWindowName(xcb_window_t w)
  {
    std::string resp;

    xcb_ewmh_get_utf8_strings_reply_t reply {};
    if (xcb_ewmh_get_wm_name_reply(
          res::ewm_connection.get(),
          xcb_ewmh_get_wm_name_unchecked(res::ewm_connection.get(), w),
          &reply,
          nullptr)) {
      resp = std::string(reply.strings, reply.strings_len);
      xcb_ewmh_get_utf8_strings_reply_wipe(&reply);
    } else if (xcb_get_property_reply_t *prop = xcb_get_property_reply(
                 res::connection.get(),
                 xcb_get_property_unchecked(res::connection.get(),
                                            0,
                                            w,
                                            XCB_ATOM_WM_NAME,
                                            XCB_ATOM_STRING,
                                            0,
                                            0),
                 nullptr);
               prop != nullptr) {
      resp = std::string(reinterpret_cast<char *>(xcb_get_property_value(prop)),
                         prop->value_len);
      delete prop;
    }

    return resp;
  }

  std::variant<XWindowInfo, std::string> getWindowInfo(xcb_window_t w)
  {
    auto                       geom = getWindowGeometry(w);
    decltype(getWindowInfo(w)) resp;

    if (std::holds_alternative<std::string>(geom)) {
      resp = std::get<std::string>(geom);
    } else {
      resp = XWindowInfo { .name     = getWindowName(w),
                           .pos      = std::get<XGeom>(geom).pos,
                           .size     = std::get<XGeom>(geom).size,
                           .parent   = getParent(w),
                           .children = queryChildren({ w }) };
    }
    return resp;
  }
} // namespace smv::details
