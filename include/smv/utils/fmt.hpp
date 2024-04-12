#pragma once

#include "smv/events.hpp"

#include <string_view>
#include <unordered_map>

#include <spdlog/fmt/fmt.h>

// for printing events
// see https://fmt.dev/9.0.0/api.html#formatting-user-defined-types
namespace fmt {
  template<>
  struct formatter<smv::EventType>: formatter<std::string_view>
  {
    template<typename FormatContext>
    auto format(smv::EventType e, FormatContext &ctx) const
    {
      std::string_view name = "Unknown";
      switch (e) {
        case smv::EventType::WindowMove:
          name = "WindowMove";
          break;
        case smv::EventType::WindowResize:
          name = "WindowResize";
          break;
        case smv::EventType::MouseEnter:
          name = "MouseEnter";
          break;
        case smv::EventType::MouseLeave:
          name = "MouseLeave";
          break;
        case smv::EventType::MouseMove:
          name = "MouseMove";
          break;
        case smv::EventType::MouseDown:
          name = "MouseDown";
          break;
        case smv::EventType::MouseUp:
          name = "MouseUp";
          break;
        case smv::EventType::MouseWheel:
          name = "MouseWheel";
          break;
        case smv::EventType::WindowCreated:
          name = "WindowCreated";
          break;
        case smv::EventType::WindowClose:
          name = "WindowClose";
          break;
        case smv::EventType::WindowRenamed:
          name = "WindowRenamed";
          break;
        default:
          name = "None";
      }
      return formatter<std::string_view>::format(name, ctx);
    }
  };

  template<typename K, typename V>
  struct formatter<std::unordered_map<K, V>>: formatter<std::string>
  {
    template<typename FormatContext>
    auto format(const std::unordered_map<K, V> &m, FormatContext &ctx) const
      -> decltype(ctx.out())
    {
      return format_to(ctx.out(), "\n{{ {} }}", join(m.begin(), m.end(), ", "));
    }
  };
} // namespace fmt
