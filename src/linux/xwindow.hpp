#include "smv/events.hpp"
#include "smv/window.hpp"
#include <list>
#include <unordered_map>
#include <utility>

namespace smv::impl {
  class XWindow final : public smv::Window
  {
    friend class XWindowsClient;

  public:
    explicit XWindow(std::uint32_t id,
                     std::int32_t x,
                     std::int32_t y,
                     std::uint32_t w,
                     std::uint32_t h)
      : mId(id)
      , mPosX(x)
      , mPosY(y)
      , mWidth(w)
      , mHeight(h)
    {
    }

    std::uint32_t id() const override { return mId; }
    std::string name() const override { return mName; }
    Position position() const override { return { .x = mPosX, .y = mPosY }; }
    Size size() const override { return { .w = mWidth, .h = mHeight }; }

  protected:
    void move(std::int32_t x, std::int32_t y)
    {
      mPosX = x;
      mPosY = y;
    }

    void resize(std::uint32_t w, std::uint32_t h)
    {
      mWidth = w;
      mHeight = h;
    }

    void setName(const std::string &name) { this->mName = name; }

  private:
    const std::uint32_t mId;
    std::int32_t mPosX, mPosY;
    std::uint32_t mWidth, mHeight;
    std::string mName;
  };

} // namespace smv::impl
