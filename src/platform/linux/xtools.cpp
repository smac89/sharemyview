#include "xtools.hpp"
#include "smv/log.hpp"
#include "xutils.hpp"

#include <memory>

#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>

static std::shared_ptr<xcb_errors_context_t> err_ctx;

namespace smv::details {
  using smv::utils::res, smv::log::logger;

  auto initTools() -> bool
  {
    if (err_ctx) {
      return true;
    }
    xcb_errors_context_t *ctx = nullptr;
    if (xcb_errors_context_new(res::connection.get(), &ctx) < 0) {
      logger->error("error creating xcb_errors_context");
      return false;
    }
    err_ctx.reset(ctx, &xcb_errors_context_free);
    return err_ctx != nullptr;
  }

  void deinitTools()
  {
    // TODO: is there a special method for freeing err_ctx?
    err_ctx.reset();
  }

  auto getErrorCodeName(uint8_t error_code) -> std::string
  {
    return xcb_errors_get_name_for_error(err_ctx.get(), error_code, nullptr);
  }

  auto getEventName(xcb_generic_event_t *const event) -> std::string
  {
    if (event == nullptr) {
      return "";
    }
    return xcb_errors_get_name_for_xcb_event(err_ctx.get(), event, nullptr);
  }
} // namespace smv::details
