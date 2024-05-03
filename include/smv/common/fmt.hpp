#pragma once

#include "smv/events.hpp"
#include "smv/record.hpp"

#include <string_view>
#include <type_traits>
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

  template<typename T, typename Char>
  struct formatter<
    T,
    Char,
    std::enable_if_t<std::is_convertible_v<T *, smv::EventData *>>>
    : formatter<std::string>
  {
    template<typename FormatContext>
    auto format(const smv::EventData &data, FormatContext &ctx) const
      -> decltype(ctx.out())
    {
      return format_to(ctx.out(), "{}", data.format());
    }
  };

  template<typename K, typename V>
  struct formatter<std::unordered_map<K, V>>: formatter<std::string>
  {
    template<typename FormatContext>
    auto format(const std::unordered_map<K, V> &map, FormatContext &ctx) const
      -> decltype(ctx.out())
    {
      return format_to(
        ctx.out(), "\n{{ {} }}", join(map.begin(), map.end(), ", "));
    }
  };

  template<>
  struct formatter<smv::ScreenshotFormat>: formatter<std::string_view>
  {
    template<typename FormatContext>
    auto format(smv::ScreenshotFormat ssFmt, FormatContext &ctx) const
    {
      std::string_view name = "Unknown";
      switch (ssFmt) {
        case smv::ScreenshotFormat::JPEG:
          name = "Jpeg";
          break;
        case smv::ScreenshotFormat::PNG:
          name = "Png";
          break;
        case smv::ScreenshotFormat::PPM:
          name = "Ppm";
          break;
        case smv::ScreenshotFormat::QOI:
          name = "Qoi";
          break;
      }
      return formatter<std::string_view>::format(name, ctx);
    }
  };
} // namespace fmt
