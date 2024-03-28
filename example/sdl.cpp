#include "indicator-win.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

static void
draw_app_windows(const std::vector<std::shared_ptr<IndicatorWin>> &);

static void update_app_windows(std::vector<std::shared_ptr<IndicatorWin>> &);

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    return EXIT_FAILURE;
  }
  std::atexit(SDL_Quit);

  std::vector<std::shared_ptr<IndicatorWin>> windows;
  update_app_windows(windows);
  draw_app_windows(windows);

  for (SDL_Event evt;;) {
    if (!SDL_PollEvent(&evt)) {
      SDL_Delay(100);
      continue;
    };
    if (evt.type == SDL_QUIT) {
      break;
    }
    switch (evt.type) {
    case SDL_DISPLAYEVENT:
      if (evt.display.event == SDL_DISPLAYEVENT_DISCONNECTED) {
        SDL_Log("disconnected display: %d", evt.display.display);
      } else if (evt.display.event == SDL_DISPLAYEVENT_CONNECTED) {
        SDL_Log("connected display: %d", evt.display.display);
      }
      update_app_windows(windows);
      break;
    case SDL_WINDOWEVENT:
      if (evt.window.event == SDL_WINDOWEVENT_CLOSE) {
        std::remove_if(windows.begin(), windows.end(), [&evt](auto &w) {
          return w && w->id() == evt.window.windowID;
        });
      }
      break;
    case SDL_MOUSEMOTION:
      SDL_Log("mouse motion: %d, %d", evt.motion.x, evt.motion.y);
      break;
    }
    draw_app_windows(windows);
  }

  windows.clear();

  return 0;
}

static void
draw_app_windows(const std::vector<std::shared_ptr<IndicatorWin>> &windows) {
  for (auto &w : windows) {
    if (w) {
      w->draw();
    }
  }
}

static void
update_app_windows(std::vector<std::shared_ptr<IndicatorWin>> &windows) {
  windows.clear();
  std::size_t num_disp = SDL_GetNumVideoDisplays();
  if (num_disp > windows.size()) {
    windows.resize(num_disp);
  }
  SDL_Rect r;
  for (std::size_t i = 0; i < num_disp; i++) {
    if (SDL_GetDisplayBounds(i, &r) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "SDL_GetDisplayBounds failed: %s", SDL_GetError());
      continue;
    }
    std::ostringstream oss;
    oss << "Capture Window " << i;
    windows.push_back(std::make_shared<IndicatorWin>(
        oss.str().c_str(), r.x, r.y, 500, 500,
        SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_SKIP_TASKBAR |
            SDL_WINDOW_MOUSE_CAPTURE));
    if (!windows.back()) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "SDL_CreateWindow (%ld) failed: %s", i, SDL_GetError());
      windows.pop_back();
      continue;
    }
  }
}
