#pragma once

#include <cstdint>
#include <string>

namespace smv {
  struct Position
  {
    int x = 0, y = 0;
  };

  struct Size
  {
    uint32_t w = 0, h = 0;
  };

  /**
   * @brief A region could be a window on the screen
   * or a part of the screen
   */
  struct Region
  {
    Region() = default;
    Region(uint32_t w, uint32_t h, int x, int y)
      : mWidth(w)
      , mHeight(h)
      , mPosX(x)
      , mPosY(y)
    {
    }
    virtual auto position() const -> Position
    {
      return { .x = mPosX, .y = mPosY };
    }

    virtual inline auto x() const -> int { return mPosX; }
    virtual inline auto y() const -> int { return mPosY; }
    virtual inline auto size() const -> Size
    {
      return { .w = mWidth, .h = mHeight };
    }
    virtual inline auto width() const -> uint32_t { return mWidth; }
    virtual inline auto height() const -> uint32_t { return mHeight; }
    virtual inline auto isValid() const -> bool
    {
      return width() > 0 && height() > 0;
    }

    virtual ~Region() = default;

  protected:
    uint32_t mWidth = 0, mHeight = 0;
    int      mPosX = 0, mPosY = 0;
  };

  struct Window: Region
  {
    using Region::Region; // inherit the constructor
    virtual auto id() const -> uint32_t      = 0;
    virtual auto name() const -> std::string = 0;
  };
} // namespace smv
