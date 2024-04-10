#pragma once

#include "smv/events.hpp"
#include "smv/window.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

constexpr auto LOGGER_NAME_WINCLIENT = "smv::winclient";

namespace smv {
  void   init() noexcept;
  void   deinit() noexcept;
  Cancel listen(EventType, EventCB);

  template<EventType E>
  constexpr Cancel listen(EventCB cb)
  {
    return listen(E, std::move(cb));
  }

  template<typename D>
  constexpr Cancel listen(TEventCB<D> cb)
  {
    return listen<D::type>([cb = std::move(cb)](const EventData &d) {
      cb(dynamic_cast<const D &>(d));
    });
  }

  template<typename D>
  constexpr Cancel listen(const std::uint32_t wid, TEventCB<D> cb)
  {
    return listen<D>([cb = std::move(cb), wid](const D &e) {
      if (auto window = e.window.lock(); window && window->id() == wid) {
        cb(e);
      }
    });
  }
} // namespace smv
