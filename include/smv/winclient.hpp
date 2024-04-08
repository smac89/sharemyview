#pragma once
#include "smv/events.hpp"
#include "smv/window.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

constexpr auto LOGGER_NAME = "smv::winclient";

namespace smv
{
  using Cancel  = std::function<void()>;
  using EventCB = std::function<void(const smv::EventData &)>;

  void   init() noexcept;
  void   deinit() noexcept;
  Cancel listen(EventType, const EventCB cb);
  Cancel listen(EventType, const std::uint32_t wid, const EventCB cb);
} // namespace smv
