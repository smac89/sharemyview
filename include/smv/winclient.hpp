#pragma once

#include "smv/events.hpp"
#include "smv/utils/fmt.hpp"
#include "smv/window.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

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
  constexpr Cancel listen(TEventCB<D> cb)
  {
    static_assert(std::is_base_of_v<smv::EventData, D>,
                  "Data must inherit from EventData");
    static_assert(E == D::type);

    return listen(E, [cb](const EventData &d) {
      logger->info("{}", d);
      cb(dynamic_cast<const D &>(d));
    });
  }

  template<smv::EventType E, typename F, typename D = Arg0<F>>
  constexpr Cancel listen(F arg)
  {
    return listen<E, D>(std::forward<F>(arg));
  }

  template<smv::EventType E, typename F, typename D = Arg0<F>>
  constexpr Cancel listen(const std::uint32_t wid, F arg)
  {
    return listen<E, D>([arg, wid](const D &e) {
      if (auto window = e.window.lock(); window && window->id() == wid) {
        arg(e);
      }
    });
  }
} // namespace smv
