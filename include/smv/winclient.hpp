#pragma once
#include "smv/events.hpp"
#include "smv/window.hpp"
#include <cstdint>
#include <memory>
#include <vector>

constexpr auto LOGGER_NAME = "smv::winclient";

namespace smv
{
  void init() noexcept;
  void deinit() noexcept;
  void listen(EventType, const EventCallback cb);
  void listen(EventType, const std::uint32_t wid, const EventCallback cb);
} // namespace smv
