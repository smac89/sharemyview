#pragma once

#include <cstdint>
#include <string>

namespace smv {
  struct Position
  {
    int x, y;
  };

  struct Size
  {
    uint32_t w, h;
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

    virtual auto x() const -> int { return mPosX; }
    virtual auto y() const -> int { return mPosY; }
    virtual auto size() const -> Size { return { .w = mWidth, .h = mHeight }; }
    virtual auto width() const -> uint32_t { return mWidth; }
    virtual auto height() const -> uint32_t { return mHeight; }

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
