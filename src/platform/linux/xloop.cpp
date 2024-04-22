#include "xloop.hpp"
#include "smv/log.hpp"
#include "xevents.hpp"
#include "xmonitor.hpp"
#include "xtools.hpp"
#include "xutils.hpp"

#include <cstdint>
#include <memory>
#include <optional>

#include <xcb/xcb_event.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xproto.h>

namespace smv::details {
  using smv::utils::res, smv::log::logger;

  void pollEvents()
  {
    auto &xevents = XEvents::instance();
    for (std::shared_ptr<xcb_generic_event_t> event(
           xcb_poll_for_event(res::connection.get()));
         event != nullptr;
         event.reset(xcb_poll_for_event(res::connection.get()))) {

      if (event->response_type == XCB_NONE) {
        auto err = std::reinterpret_pointer_cast<xcb_generic_error_t>(event);
        logger->error("Error: {}",
                      xcb_event_get_error_label(err->response_type));
      }
      // See https://www.x.org/wiki/Development/Documentation/XGE/
      else if (event->response_type == XCB_GE_GENERIC) {
        auto gevent =
          std::reinterpret_pointer_cast<xcb_ge_generic_event_t>(event);
        logger->info("Received generic event {}", getEventName(event.get()));
      } else {
        switch (XCB_EVENT_RESPONSE_TYPE(event)) {
            /*
             * NOTE: we only care about the fact that a window is entered
             * or left. Whether or not it used to be inferior to another window
             * is irrelevant.
             *
             * See:
             * https://www.x.org/releases/current/doc/xproto/x11protocol.html#events:pointer_window
             *
             */
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
                  xevents.getWatchWindow(configure->window).lock()) {
              if (window->position().x != configure->x ||
                  window->position().y != configure->y) {
                xevents.onWindowMoved(
                  configure->window, configure->x, configure->y);
              }
              if (window->size().w != configure->width ||
                  window->size().h != configure->height) {
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
            logger->debug("Received default event: {}",
                          getEventName(event.get()));
            break;
          }
        }
      }
    }
  }

  auto windowIsNormalType(const xcb_window_t window) -> bool
  {
    auto window_type_req =
      xcb_ewmh_get_wm_window_type_unchecked(res::ewm_connection.get(), window);
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

  auto getParent(xcb_window_t window) -> std::optional<xcb_window_t>
  {
    xcb_query_tree_reply_t *reply = xcb_query_tree_reply(
      res::connection.get(),
      xcb_query_tree_unchecked(res::connection.get(), window),
      nullptr);
    if (reply->parent == XCB_NONE) {
      return std::nullopt;
    }
    return reply->parent;
  }

  auto getWindowGeometry(xcb_window_t window)
    -> std::variant<XGeom, std::string>
  {
    xcb_generic_error_t                      *err = nullptr;
    std::shared_ptr<xcb_get_geometry_reply_t> reply(
      xcb_get_geometry_reply(res::connection.get(),
                             xcb_get_geometry(res::connection.get(), window),
                             &err));
    std::shared_ptr<xcb_generic_error_t> _ { err };

    decltype(getWindowGeometry(window)) resp;

    if (err != nullptr) {
      resp = getErrorCodeName(err->error_code);
    } else {
      resp = XGeom { .pos  = { .x = reply->x, .y = reply->y },
                     .size = { .w = reply->width, .h = reply->height } };
    }
    return resp;
  }

  auto getWindowName(xcb_window_t window) -> std::string
  {
    static const auto CHUNK_SIZE = 100;

    std::string resp;
    for (uint32_t offset = 0; offset % CHUNK_SIZE == 0; offset += CHUNK_SIZE) {
      std::shared_ptr<xcb_get_property_reply_t> prop(
        xcb_get_property_reply(res::connection.get(),
                               xcb_get_property_unchecked(res::connection.get(),
                                                          0,
                                                          window,
                                                          XCB_ATOM_WM_NAME,
                                                          XCB_ATOM_STRING,
                                                          offset,
                                                          CHUNK_SIZE),
                               nullptr));
      if (prop == nullptr) {
        logger->warn("Failed to get WM_NAME for window: {:#x}", window);
        break;
      }
      auto length = xcb_get_property_value_length(prop.get());
      resp.append(static_cast<char *>(xcb_get_property_value(prop.get())),
                  length);
      if (length < CHUNK_SIZE) {
        logger->debug("Found window WM_NAME: '{}'", resp);
        return resp;
      }
    }

    xcb_ewmh_get_utf8_strings_reply_t reply {};
    if (xcb_ewmh_get_wm_name_reply(
          res::ewm_connection.get(),
          xcb_ewmh_get_wm_name_unchecked(res::ewm_connection.get(), window),
          &reply,
          nullptr) ||
        xcb_ewmh_get_wm_visible_name_reply(
          res::ewm_connection.get(),
          xcb_ewmh_get_wm_visible_name_unchecked(res::ewm_connection.get(),
                                                 window),
          &reply,
          nullptr)) {
      resp = std::string(reply.strings, reply.strings_len);
      logger->debug("Found window _NET_WM_NAME: '{}'", resp);
      xcb_ewmh_get_utf8_strings_reply_wipe(&reply);
    } else {
      logger->warn("Failed to get _NET_WM_NAME for window: {:#x}", window);
    }
    return resp;
  }

  auto getWindowInfo(xcb_window_t window)
    -> std::variant<XWindowInfo, std::string>
  {
    auto                            geom = getWindowGeometry(window);
    decltype(getWindowInfo(window)) resp;

    if (std::holds_alternative<std::string>(geom)) {
      resp = std::get<std::string>(geom);
    } else {
      resp = XWindowInfo { .name     = getWindowName(window),
                           .pos      = std::get<XGeom>(geom).pos,
                           .size     = std::get<XGeom>(geom).size,
                           .parent   = getParent(window),
                           .children = queryChildren({ window }) };
    }
    return resp;
  }
} // namespace smv::details
