#pragma once

#include "events.hpp"
#include "log.hpp"

#include <cstdint>
#include <functional>
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

namespace smv {
  using smv::log::logger;

  /**
   * @brief Initialize the platform-specific backend
   */
  void init() noexcept;

  /**
   * @brief Deinitialize the platform-specific backend
   */
  void deinit() noexcept;

  /**
   * @brief register a callback to receive an event callback
   * @details the callback is called when an event occurs
   *
   * @return Cancel A function for cancelling the subscription
   */
  auto listen(EventType, EventCB) -> Cancel;

  template<EventType E, typename D>
  inline auto listen(TEventCB<D> func) -> Cancel
  {
    static_assert(std::is_base_of_v<EventData, D>,
                  "Data must inherit from EventData");
    static_assert(E == D::type,
                  "Missing type field. Event type must match data type");

    return listen(E, [func = std::move(func)](const EventData &data) {
      func(dynamic_cast<const D &>(data));
    });
  }

  template<EventType E, typename F, typename D = Arg0<F>>
  inline auto listen(F func) -> Cancel
  {
    return listen<E, D>(std::forward<F>(func));
  }

  template<EventType E, typename F, typename D = Arg0<F>>
  inline auto listen(const uint32_t wid, F func) -> Cancel
  {
    return listen<E, D>([func, wid](const D &data) {
      if (auto window = data.window.lock(); window && window->id() == wid) {
        func(data);
      }
    });
  }
} // namespace smv
