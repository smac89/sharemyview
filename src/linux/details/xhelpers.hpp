#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include <xcb/xcb.h>

namespace smv::details
{
  bool initHelpers();
  void deinitHelpers();

  std::string getErrorString(uint8_t error_code);

  std::string getEventName(const std::weak_ptr<xcb_generic_event_t> &event);
} // namespace smv::details
