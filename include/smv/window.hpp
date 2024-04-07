#pragma once
#include "smv/events.hpp"
#include <cstdint>
#include <string>

namespace smv
{
  struct Position
  {
    std::int32_t x, y;
  };

  struct Size
  {
    std::uint32_t w, h;
  };

  struct Window
  {
    virtual std::uint32_t id() const       = 0;
    virtual std::string   name() const     = 0;
    virtual Position      position() const = 0;
    virtual Size          size() const     = 0;
    virtual ~Window()                      = default;
  };
} // namespace smv
