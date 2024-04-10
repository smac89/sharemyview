#pragma once
#include "smv/events.hpp"

#include <cstddef>

#include <spdlog/fmt/fmt.h>

namespace {
  template<typename T>
  struct CPterIteratorSentinel
  {
    T *const    mRef;
    std::size_t mIndex;

    T                        &operator*() { return mRef[mIndex]; }
    CPterIteratorSentinel<T> &operator++()
    {
      ++mIndex;
      return *this;
    }
    bool operator!=(const CPterIteratorSentinel<T> &other) const
    {
      return mIndex != other.mIndex;
    }
  };
} // namespace

namespace smv::utils {
  template<typename T>
  class CPtrIterator
  {
  public:
    explicit CPtrIterator(T *const ptr, const std::size_t size)
      : mPtr(ptr)
      , mSize(size)
    {
    }

    CPterIteratorSentinel<T> begin() { return { mPtr, 0 }; }
    CPterIteratorSentinel<T> end() { return { nullptr, mSize }; }

  private:
    T *const          mPtr;
    const std::size_t mSize;
  };
} // namespace smv::utils

// for printing events
// see https://fmt.dev/9.0.0/api.html#formatting-user-defined-types
template<>
struct fmt::formatter<smv::EventType>: formatter<std::string_view>
{
  template<typename FormatContext>
  auto format(smv::EventType e, FormatContext &ctx) const
  {
    std::string_view name = "Unknown";
    switch (e) {
      case smv::EventType::Move:
        name = "Move";
        break;
      case smv::EventType::Resize:
        name = "Resize";
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
