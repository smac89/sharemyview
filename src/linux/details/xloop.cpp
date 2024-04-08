#include "xloop.hpp"
#include "xevents.hpp"
#include "xtools.hpp"
#include "xutils.hpp"

#include <xcb/xcb_ewmh.h>

namespace smv::details
{
  using smv::utils::res;

  void pollEvents()
  {
    std::shared_ptr<xcb_generic_event_t> event;
    auto                                &xevents = XEvents::getInstance();
    for (event.reset(xcb_poll_for_event(res::connection.get()));
         event != nullptr;
         event.reset(xcb_poll_for_event(res::connection.get())))
    {

      if (event->response_type == XCB_NONE)
      {
        auto err = std::reinterpret_pointer_cast<xcb_generic_error_t>(event);
        if (err->error_code != XCB_WINDOW)
        {
          // XCB_WINDOW is the error code for an invalid window
          res::logger->error("Received error: {}",
                             getErrorCodeName(err->error_code));
        }
      }
      // See https://www.x.org/wiki/Development/Documentation/XGE/
      else if (event->response_type == XCB_GE_GENERIC)
      {
        auto gevent =
          std::reinterpret_pointer_cast<xcb_ge_generic_event_t>(event);
        res::logger->info("Received generic event {}",
                          getEventName(event.get()));
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
              xevents.onWindowEntered(enter->event);
            }
            break;
          }
          case XCB_LEAVE_NOTIFY:
          {
            auto leave =
              std::reinterpret_pointer_cast<xcb_leave_notify_event_t>(event);
            if (leave->detail != XCB_NOTIFY_DETAIL_INFERIOR)
            {
              xevents.onWindowLeave(leave->event);
            }
            break;
          }
          case XCB_CREATE_NOTIFY:
          {
            auto create =
              std::reinterpret_pointer_cast<xcb_create_notify_event_t>(event);
            xevents.onWindowCreated(create->window, create->parent);
            break;
          }
          case XCB_DESTROY_NOTIFY:
          {
            auto destroy =
              std::reinterpret_pointer_cast<xcb_destroy_notify_event_t>(event);
            xevents.onWindowDestroyed(destroy->window);
            break;
          }
          default:
          {
            res::logger->info("Received default event: {}",
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
          res::ewm_connection.get(), window_type_req, &atom_reply, nullptr))
    {
      return false;
    }

    //  created as a way to free the reply once the function returns
    std::unique_ptr<xcb_ewmh_get_atoms_reply_t,
                    decltype(&xcb_ewmh_get_atoms_reply_wipe)>
      deferred(&atom_reply, &xcb_ewmh_get_atoms_reply_wipe);

    return atom_reply.atoms_len > 0 &&
           *atom_reply.atoms == res::ewm_connection->_NET_WM_WINDOW_TYPE_NORMAL;
  }

  std::variant<XGeom, std::string_view> getWindowGeometry(xcb_window_t w)
  {
    xcb_generic_error_t                      *err = nullptr;
    std::shared_ptr<xcb_get_geometry_reply_t> reply(xcb_get_geometry_reply(
      res::connection.get(), xcb_get_geometry(res::connection.get(), w), &err));
    std::shared_ptr<xcb_generic_error_t>      _ { err };

    decltype(getWindowGeometry(w)) resp;

    if (err)
    {
      resp = getErrorCodeName(err->error_code);
    }
    else
    {
      resp = XGeom { .pos  = { reply->x, reply->y },
                     .size = { reply->width, reply->height } };
    }
    return resp;
  }

} // namespace smv::details
