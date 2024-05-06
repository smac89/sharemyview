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

  // Note that usage of this template may require explicit instantiation
  template<EventType E>
  void sendRequest(uint32_t wid, const EventData &data, EventCB callback);

  /**
   * @brief send an event
   * @details the event is sent to the native window manager. The callback is
   * only called once then it is automatically unsubscribed
   * @param type the type of event
   * @param wid the id of the window to send the event to
   * @param data the data to send
   * @param func the function to call when the event occurs
   */
  template<typename D>
  inline void sendRequest(const uint32_t wid,
                          const D       &data,
                          TEventCB<D>    func) noexcept
  {
    static_assert(std::is_base_of_v<EventData, D>,
                  "Data must inherit from EventData");

    sendRequest<D::type>(
      wid, data, [func = std::move(func)](const EventData &data) {
      func(dynamic_cast<const D &>(data));
    });
  }

  template<typename D, typename F>
  inline void sendRequest(const uint32_t wid, const D &data, F func)
  {
    return sendRequest<D>(
      wid,
      data,
      std::forward<TEventCB<D>>([func = std::move(func)](const D &data) {
      func(data);
    }));
  }

  template<typename D, typename F>
  inline void sendRequest(const D &data, F func)
  {
    return sendRequest<D, F>(
      data.window.lock()->id(), data, std::forward<F>(func));
  }
} // namespace smv
