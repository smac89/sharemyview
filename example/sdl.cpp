#include "indicator-win.hpp"
#include <GLFW/glfw3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <algorithm>
#include <cstdlib>
#include <hello_imgui/hello_imgui.h>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <sstream>
#include <vector>

// static void
// draw_app_windows(const std::vector<std::shared_ptr<IndicatorWin>> &);

// static void
// update_app_windows(std::vector<std::shared_ptr<IndicatorWin>> &);

int
main()
{
  auto guiFunction = []() {
    ImGui::Text("Hello, ");                  // Display a simple label
    HelloImGui::ImageFromAsset("world.jpg"); // Display a static image
    if (ImGui::Button("Bye!"))               // Display a button
      // and immediately handle its action if it is clicked!
      HelloImGui::GetRunnerParams()->appShallExit = true;
  };
  HelloImGui::Run(guiFunction, "Hello, globe", true);
  // if (SDL_Init(SDL_INIT_VIDEO) < 0) {
  //   return EXIT_FAILURE;
  // }
  // std::atexit(SDL_Quit);
  // // std::vector<std::shared_ptr<IndicatorWin>> windows;
  // std::shared_ptr<SDL_Window> pointerWindow(
  //   SDL_CreateWindow("Pointer Window",
  //                    SDL_WINDOWPOS_CENTERED,
  //                    SDL_WINDOWPOS_CENTERED,
  //                    640,
  //                    480,
  //                    SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL |
  //                      SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_SKIP_TASKBAR),
  //   &SDL_DestroyWindow);

  // std::shared_ptr<SDL_Renderer> renderer(
  //   SDL_CreateRenderer(pointerWindow.get(),
  //                      -1,
  //                      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
  //   &SDL_DestroyRenderer);

  // if (renderer == nullptr) {
  //   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
  //                "SDL_CreateRenderer failed: %s",
  //                SDL_GetError());
  //   std::exit(EXIT_FAILURE);
  // }

  // if (SDL_SetWindowOpacity(pointerWindow.get(), 0) < 0) {
  //   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
  //                "SDL_SET_WindownOpacity failed: %s",
  //                SDL_GetError());
  // }

  // float opacity = 0.0f;
  // SDL_GetWindowOpacity(pointerWindow.get(), &opacity);
  // if (opacity > 0.0f) {
  //   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
  //                "SDL_GetWindowOpacity failed: %s",
  //                SDL_GetError());
  // }

  // auto ctx = SDL_GL_CreateContext(pointerWindow.get());
  // if (ctx == nullptr) {
  //   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
  //                "SDL_GL_CreateContext failed: %s",
  //                SDL_GetError());
  //   std::exit(EXIT_FAILURE);
  // }

  // SDL_GL_MakeCurrent(pointerWindow.get(), ctx);
  // // SDL_GL_SetSwapInterval(1);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glEnable(GL_BLEND);

  // // SDL_GL_SwapWindow(pointerWindow.get());

  // // if (SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND) < 0)
  // {
  // //   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
  // //                "SDL_SetRenderDrawBlendMode failed: %s",
  // //                SDL_GetError());
  // // }

  // // SDL_CreateWindowFrom()

  // // SDL_GetWindowSize(pointerWindow.get(), &rect.w, &rect.h);
  // // SDL_SetRenderDrawColor(renderer.get(), 32, 32, 32, SDL_ALPHA_OPAQUE);
  // // SDL_RenderClear(renderer.get());
  // // SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, SDL_ALPHA_TRANSPARENT);
  // // SDL_RenderFillRect(renderer.get(), nullptr);

  // // SDL_RenderPresent(renderer.get());
  // // SDL_GL_SwapWindow(pointerWindow.get());

  // for (SDL_Event evt;;) {
  //   if (!SDL_PollEvent(&evt)) {
  //     SDL_Delay(10);
  //     continue;
  //   };
  //   if (evt.type == SDL_QUIT) {
  //     spdlog::info("Exiting...");
  //     break;
  //   }

  //   glClearColor(0.f, 0.f, 0.f, 0.f);
  //   glClear(GL_COLOR_BUFFER_BIT);

  //   // SDL_GL_SwapWindow(pointerWindow.get());

  //   // SDL_GetWindowPosition(pointerWindow.get(), &rect.x, &rect.y);
  //   // SDL_SetRenderDrawColor(renderer.get(), 32, 32, 32, SDL_ALPHA_OPAQUE);
  //   // SDL_RenderClear(renderer.get());
  //   // SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0,
  //   SDL_ALPHA_TRANSPARENT);
  //   // SDL_RenderFillRect(renderer.get(), nullptr);
  //   // SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0,
  //   SDL_ALPHA_TRANSPARENT);
  //   // if (SDL_RenderFillRect(renderer.get(), nullptr) < 0) {
  //   //   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
  //   //                "SDL_RenderFillRect failed: %s",
  //   //                SDL_GetError());
  //   // }
  //   // SDL_RenderPresent(renderer.get());
  //   // SDL_GL_SwapWindow(pointerWindow.get());
  // }

  // SDL_GL_DeleteContext(ctx);
  // SDL_Quit();
  return 0;
}

// static void
// draw_app_windows(const std::vector<std::shared_ptr<IndicatorWin>> &windows)
// {
//   for (auto &w : windows) {
//     if (w) {
//       w->draw();
//     }
//   }
// }

// static void
// update_app_windows(std::vector<std::shared_ptr<IndicatorWin>> &windows)
// {
//   windows.clear();
//   std::size_t num_disp = SDL_GetNumVideoDisplays();
//   if (num_disp > windows.size()) {
//     windows.resize(num_disp);
//   }
//   SDL_Rect r;
//   for (std::size_t i = 0; i < num_disp; i++) {
//     if (SDL_GetDisplayBounds(i, &r) < 0) {
//       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
//                    "SDL_GetDisplayBounds failed: %s",
//                    SDL_GetError());
//       continue;
//     }
//     std::ostringstream oss;
//     oss << "Capture Window " << i;
//     windows.push_back(std::make_shared<IndicatorWin>(
//       oss.str().c_str(),
//       r.x,
//       r.y,
//       500,
//       500,
//       SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_SKIP_TASKBAR |
//         SDL_WINDOW_MOUSE_CAPTURE));
//     if (!windows.back()) {
//       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
//                    "SDL_CreateWindow (%ld) failed: %s",
//                    i,
//                    SDL_GetError());
//       windows.pop_back();
//       continue;
//     }
//   }
// }
