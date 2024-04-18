#include "app/smv_app.hpp"
#include "app/smv_image_provider.hpp"
#include "smv/winclient.hpp"

#include <csignal>
#include <cstdlib>
#include <memory>

#include <QGuiApplication>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
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
  engine.addImageProvider("smv", new AppImageProvider);
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

namespace smv::log {
  extern std::shared_ptr<spdlog::logger> const logger =
    spdlog::stderr_color_mt(LOGGER_NAME_WINCLIENT);
}
