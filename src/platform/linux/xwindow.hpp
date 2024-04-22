#pragma once
#include "smv/window.hpp"

#include <optional>
#include <vector>
#include <xcb/xcb.h>

namespace smv::details {
  class XWindow final: public smv::Window
  {
    friend class XEvents;

  public:
    explicit XWindow(uint32_t id, int32_t x, int32_t y, uint32_t w, uint32_t h)
      : smv::Window(w, h, x, y)
      , mId(id)
    {
    }
    auto id() const -> uint32_t override { return mId; }
    auto name() const -> std::string override { return mName; }
    auto children() const -> std::vector<xcb_window_t> { return mChildren; }

  protected:
    void move(int32_t x, int32_t y)
    {
      mPosX = x;
      mPosY = y;
    }

    void resize(uint32_t w, uint32_t h)
    {
      mWidth  = w;
      mHeight = h;
    }

    void setName(const std::string &name) { this->mName = name; }
    void setParent(const xcb_window_t *const parent)
    {
      if (parent != nullptr) {
        this->mParent = *parent;
      } else {
        this->mParent = std::nullopt;
      }
    }
    void addChild(xcb_window_t child) { this->mChildren.push_back(child); }
    void addChildren(const std::vector<xcb_window_t> &children)
    {
      this->mChildren.insert(
        this->mChildren.end(), children.begin(), children.end());
    }

  private:
    uint32_t                    mId;
    std::string                 mName;
    std::optional<xcb_window_t> mParent;
    std::vector<xcb_window_t>   mChildren;
  };
} // namespace smv::details
