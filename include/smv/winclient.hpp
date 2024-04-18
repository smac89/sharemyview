#pragma once

#include "smv/events.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include <spdlog/spdlog.h>

namespace {
  template<std::size_t n, typename F>
  struct arg_n;

  template<std::size_t n, typename R, typename... Ts>
  struct arg_n<n, std::function<R(Ts...)>>
  {
    using type = std::tuple_element_t<n, std::tuple<Ts...>>;
  };

  template<typename F>
  using Arg0 = std::remove_cv_t<std::remove_reference_t<
    typename arg_n<0, decltype(std::function { std::declval<F>() })>::type>>;
} // namespace

namespace smv::log {
  constexpr auto LOGGER_NAME_WINCLIENT = "smv::winclient";
  extern const std::shared_ptr<spdlog::logger> logger;
} // namespace smv::log

namespace smv {
  using smv::log::logger;

  void   init() noexcept;
  void   deinit() noexcept;
  Cancel listen(EventType, EventCB);

  template<EventType E, typename D>
  inline Cancel listen(TEventCB<D> fn)
  {
    static_assert(std::is_base_of_v<EventData, D>,
                  "Data must inherit from EventData");
    static_assert(E == D::type,
                  "Missing type field. Event type must match data type");

    return listen(E, [fn = std::move(fn)](const EventData &d) {
      fn(dynamic_cast<const D &>(d));
    });
  }

  template<EventType E, typename F, typename D = Arg0<F>>
  inline Cancel listen(F fn)
  {
    return listen<E, D>(std::forward<F>(fn));
  }

  template<EventType E, typename F, typename D = Arg0<F>>
  inline Cancel listen(const std::uint32_t wid, F fn)
  {
    return listen<E, D>([fn, wid](const D &e) {
      if (auto window = e.window.lock(); window && window->id() == wid) {
        fn(e);
      }
    });
  }
} // namespace smv
