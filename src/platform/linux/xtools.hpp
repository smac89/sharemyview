#pragma once
#include <cstdint>
#include <string>

#include <xcb/xcb.h>

namespace smv::details {
  auto initTools() -> bool;
  void deinitTools();

  auto getErrorCodeName(uint8_t error_code) -> std::string;
  auto getEventName(xcb_generic_event_t *) -> std::string;
} // namespace smv::details
