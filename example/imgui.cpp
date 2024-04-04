#include "app/smv_app.hpp"
#include "smv/winclient.hpp"
#include <GLFW/glfw3.h>
#define GLFW_NATIVE_INCLUDE_NONE
#ifdef __linux__
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>
#include <csignal>
#include <functional>
#include <hello_imgui/hello_imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

static void sigHandler(int);

int
main()
{
  spdlog::cfg::load_env_levels();
  std::signal(SIGINT, sigHandler);
  std::signal(SIGTERM, sigHandler);

  spdlog::info("Starting...");
  smv::init();

  AppState state;
  HelloImGui::RunnerParams params;
  params.appWindowParams.windowTitle = "Share My View";
  params.appWindowParams.borderlessClosable = true;
  params.imGuiWindowParams.enableViewports = true;
  params.imGuiWindowParams.backgroundColor = { 0.0f, 0.0f, 0.0f, 0.0f };
  params.imGuiWindowParams.showStatusBar = true;
  params.imGuiWindowParams.showStatus_Fps = true;
  params.imGuiWindowParams.defaultImGuiWindowType =
    HelloImGui::DefaultImGuiWindowType::NoDefaultWindow;
  params.appWindowParams.windowGeometry.size = { 480, 640 };
  params.appWindowParams.windowGeometry.positionMode =
    HelloImGui::WindowPositionMode::MonitorCenter;

  params.callbacks.PostInit_AddPlatformBackendCallbacks = [&params]() {
    // TODO: Register callbacks on the window
    auto window = static_cast<GLFWwindow *>(params.backendPointers.glfwWindow);
    glfwSetWindowAttrib(window, GLFW_VISIBLE, GLFW_TRUE);
    glfwSetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    // glClearColor(0.251f, 0.227f, 0.227f, 0.0f);
    // glfwSetWindowOpacity(window, 0.5f);
    // glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    spdlog::info("Adding platform backend callbacks...");
  };
  params.callbacks.SetupImGuiConfig = []() {
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  };
  params.callbacks.SetupImGuiStyle = [&params]() {
    // this sets the style of the imgui windows inside the app
    auto &style = ImGui::GetStyle();
    // ImVec4 clear_color = ImVec4(0.95f, 0.60f, 0.73f, 1.0f);
    ImVec4 clear_color = { 1.0f, 1.0f, 1.0f, 0.05f };
    style.Colors[ImGuiCol_WindowBg] = clear_color;
    style.Colors[ImGuiCol_ChildBg] = clear_color;
  };
  params.callbacks.ShowGui = [&state]() { showGui(state); };
  params.callbacks.PostInit = [&state]() {
    // glEnable(GL_BLEND);
    // glBlendEquation(GL_FUNC_ADD);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
    // GL_ONE);
    startApp(state);
  };
  params.callbacks.BeforeExit = [&state]() { stopApp(state); };
#ifdef SMV_DEBUG
  params.iniFolderType = HelloImGui::IniFolderType::CurrentFolder;
#else
  params.iniFolderType = HelloImGui::IniFolderType::AppUserConfigFolder;
#endif

  glfwSetErrorCallback([](int error, const char *description) {
    spdlog::error("GLFW error {}: {}", error, description);
  });

  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
  HelloImGui::Run(params);
  smv::deinit();

  spdlog::info("Finished");
  return 0;
}

void
sigHandler(int)
{
  static std::once_flag flag;
  std::call_once(flag,
                 []() { HelloImGui::GetRunnerParams()->appShallExit = true; });
}

// bool
// eventHandler(SDL_Event *event)
// {
//   if (event->type == SDL_WINDOWEVENT) {
//     switch (event->window.event) {
//       case SDL_WINDOWEVENT_MOVED:
//         spdlog::info("sdl window moved to {} {}",
//                      event->window.data1,
//                      event->window.data2);
//     }
//   } else if (event->type == SDL_MOUSEMOTION) {
//     spdlog::info("sdl mouse x: {}, y: {}", event->motion.x, event->motion.y);
//     // return true;
//   }
//   return false;
// }

// params.callbacks.CustomBackground = []() {
//   auto &io = ImGui::GetIO();
//   glViewport(0,
//              0,
//              static_cast<int>(io.DisplaySize.x),
//              static_cast<int>(io.DisplaySize.y));
//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// };
