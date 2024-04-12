#include "app/smv_app.hpp"
#include "smv/winclient.hpp"
#include <QGuiApplication>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <csignal>
#include <cstdlib>
#include <functional>
#include <memory>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

static void setupSignals();

int main(int argc, char *argv[])
{
  spdlog::cfg::load_env_levels();
  setupSignals();

  spdlog::info("Starting...");
  smv::init();

  std::atexit([]() {
    smv::deinit();
    spdlog::info("Finished");
  });

  QCoreApplication::setOrganizationName("Ubiquity");
  QCoreApplication::setApplicationName("Capture");
  QGuiApplication       app(argc, argv);
  const QUrl            mainUrl("qrc:/qml/capture.qml");
  QQmlApplicationEngine engine;

  const QMetaObject::Connection connection = QObject::connect(
    &engine,
    &QQmlApplicationEngine::objectCreated,
    &app,
    [&](QObject *root, const QUrl &url) {
    if (url != mainUrl) {
      return;
    }
    if (root == nullptr) {
      spdlog::error("Failed to load qml");
      // this does not quit the program entirely,
      // so we need an else
      QCoreApplication::exit(EXIT_FAILURE);
    } else {
      QObject::disconnect(connection);
    }
  },
    Qt::QueuedConnection);

  App smvApp;
  engine.rootContext()->setContextProperty("smvApp", &smvApp);
  engine.load(mainUrl);
  return app.exec();
}

void sigHandler(int)
{
  static std::once_flag flag;
  std::call_once(flag, &QCoreApplication::quit);
}

void setupSignals()
{
  std::signal(SIGINT, sigHandler);
  std::signal(SIGTERM, sigHandler);
  std::signal(SIGQUIT, sigHandler);
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
