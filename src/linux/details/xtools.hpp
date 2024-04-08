#pragma once
#include <cstdint>
#include <string>

#include <xcb/xcb.h>

namespace smv::details
{
  bool initTools();
  void deinitTools();

  std::string getErrorCodeName(uint8_t error_code);
  std::string getEventName(xcb_generic_event_t *const);
} // namespace smv::details
