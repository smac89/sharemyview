#include "smv/events.hpp"

#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

class Bar;

template<std::size_t n, typename F>
struct arg_n;

template<std::size_t n, typename R, typename... Ts>
struct arg_n<n, std::function<R(Ts...)>>
{
  using type = std::tuple_element_t<n, std::tuple<Ts...>>;
};

void Fns(smv::EventType E, std::function<void(const smv::EventData &)> arg)
{
  if (E == smv::EventType::MouseDown) {
    arg(smv::EventDataMouseDown {
      std::weak_ptr<smv::Window> {}, 0, 0, smv::MouseButton::Left });
  }
}

template<smv::EventType E, typename ArgD>
constexpr void Fns(std::function<void(const ArgD &)> arg)
{
  static_assert(std::is_base_of_v<smv::EventData, ArgD>,
                "Data must inherit from EventData");
  static_assert(E == ArgD::type);
  return Fns(E, [arg](const smv::EventData &d) {
    arg(dynamic_cast<const ArgD &>(d));
  });
}

template<smv::EventType E, typename F>
constexpr void Fns(F arg)
{
  using Fn = decltype(std::function { arg });

  using ArgD =
    std::remove_cv_t<std::remove_reference_t<typename arg_n<0, Fn>::type>>;
  std::cout << "ArgD: " << typeid(ArgD).name() << std::endl;
  return Fns<E, ArgD>(std::forward<F>(arg));
  // return Fns<E, ArgD>([arg](const ArgD &d) {
  //   std::invoke(arg, d);
  // });
}

template<smv::EventType E, typename ArgD>
constexpr void Fns(void (*arg)(const ArgD &))
{
  return Fns<E, ArgD>([arg](const ArgD &d) {
    arg(d);
  });
}

template<smv::EventType E, typename ArgD>
constexpr void Fns(uint32_t wid, void (*arg)(const ArgD &))
{
  return Fns<E>([wid, arg](const ArgD &d) {
    if (auto window = d.window.lock(); window && window->id() == wid) {
      arg(d);
    }
  });
}

void Foo(const smv::EventDataMouseDown &m)
{
  std::cout << "[Foo] hello mouse: " << m.x << "," << m.y << std::endl;
}

class Bar
{
  void operator()(const smv::EventDataMouseDown &m)
  {
    std::cout << "[BarO] hello mouse: " << m.x << "," << m.y << std::endl;
  }

public:
  Bar &operator=(Bar &&) = default;
  void setup()
  {
    Fns<smv::EventType::MouseDown>(
      [this](const smv::EventDataMouseDown &m) -> void {
      std::cout << "[Bar] hello mouse: " << m.x << "," << m.y << std::endl;
    });

    Fns<smv::EventType::MouseDown, smv::EventDataMouseDown>(
      std::bind(&Bar::operator(), this, std::placeholders::_1));
  }
};

auto main() -> int
{
  Fns<smv::EventType::MouseDown>(Foo);
  Bar bar;
  bar.setup();
  // std::cout << "hello" << std::endl;
  return 0;
}
