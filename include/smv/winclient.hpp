#pragma once
#include "events.hpp"
#include "windowobject.hpp"
#include <memory>
#include <vector>

namespace smv {
  void init() noexcept;
  void deinit() noexcept;
  void listen(EventType, const EventCallback &cb);
} // namespace smv
