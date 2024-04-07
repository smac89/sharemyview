#include "xhelpers.hpp"
#include "xvars.hpp"

#include <memory>

#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>

static std::shared_ptr<xcb_errors_context_t> err_ctx;

namespace smv::details
{
  using smv::common::connection;
  using smv::common::logger;

  bool initHelpers()
  {
    if (err_ctx)
    {
      return true;
    }
    xcb_errors_context_t *ctx = nullptr;
    if (xcb_errors_context_new(connection.get(), &ctx) < 0)
    {
      logger->error("error creating xcb_errors_context");
      return false;
    }
    err_ctx.reset(ctx, &xcb_errors_context_free);
    return err_ctx != nullptr;
  }

  void deinitHelpers()
  {
    // TODO: is there a special method for freeing err_ctx?
    err_ctx.reset();
  }

  std::string getErrorString(uint8_t error_code)
  {
    return xcb_errors_get_name_for_error(err_ctx.get(), error_code, nullptr);
  }

  std::string getEventName(const std::weak_ptr<xcb_generic_event_t> &event)
  {
    if (auto locked_sp = event.lock())
    {
      return xcb_errors_get_name_for_xcb_event(
        err_ctx.get(), locked_sp.get(), nullptr);
    }
    logger->error("The event is already destroyed");
    return "unknown";
  }
} // namespace smv::details
