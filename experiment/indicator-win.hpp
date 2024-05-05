#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <cstdint>
#include <memory>
#ifdef __linux__
#include <xcb/xcb.h>
#endif

class IndicatorWin
{
public:
  template<typename... Args>
  IndicatorWin(Args &&...args) noexcept
  {
    win_ = std::shared_ptr<SDL_Window>(
      SDL_CreateWindow(std::forward<Args>(args)...), &SDL_DestroyWindow);

    if (win_ == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "SDL_CreateWindow failed: %s",
                   SDL_GetError());
    }

    if (SDL_SetWindowOpacity(win_.get(), SDL_ALPHA_TRANSPARENT) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "SDL_SetWindowOpacity failed: %s",
                   SDL_GetError());
    }

    render_ = std::shared_ptr<SDL_Renderer>(
      SDL_CreateRenderer(
        win_.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
      &SDL_DestroyRenderer);

    if (render_ == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "SDL_CreateRenderer failed: %s",
                   SDL_GetError());
    }

    if (SDL_SetRenderDrawBlendMode(render_.get(), SDL_BLENDMODE_BLEND) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "SDL_SetRenderDrawBlendMode failed: %s",
                   SDL_GetError());
    }

    if (SDL_SetRenderDrawColor(render_.get(), 0, 0, 0, SDL_ALPHA_TRANSPARENT) <
        0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "SDL_SetRenderDrawColor failed: %s",
                   SDL_GetError());
    }
  }

  void draw()
  {
    if (render_ == nullptr) {
      return;
    }
    if (ticks_ == 0) {
      ticks_ = SDL_GetTicks64();
    }
    if (SDL_RenderClear(render_.get()) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "SDL_RenderClear failed: %s",
                   SDL_GetError());
    }
    // SDL_Rect r;
    // SDL_GetWindowSize(win_.get(), &r.w, &r.h);
    // SDL_GetWindowPosition(win_.get(), &r.x, &r.y);
    // if (SDL_SetRenderDrawColor(render_.get(), 255, 255, 255, 255) < 0) {
    //   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
    //                "SDL_SetRenderDrawColor failed: %s", SDL_GetError());
    // }

    // if (SDL_RenderFillRect(render_.get(), &r) < 0) {
    //   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
    //                "SDL_RenderFillRect failed: %s", SDL_GetError());
    // }

    // TODO: draw something
    SDL_RenderPresent(render_.get());
  }

  bool operator()()
  {
    return this->win_ != nullptr && this->render_ != nullptr;
  }

  uint32_t id() const { return SDL_GetWindowID(win_.get()); }

private:
  uint64_t ticks_ = 0;
  std::shared_ptr<SDL_Window> win_;
  std::shared_ptr<SDL_Renderer> render_;
};
