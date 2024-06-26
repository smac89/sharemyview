#include "app/smv_app.hpp"
#include "app/smv_image_provider.hpp"
#include "app/smv_utils.hpp"
#include "smv/client.hpp"

#include <csignal>
#include <cstdlib>
#include <memory>

#include <QApplication>
#include <QLoggingCategory>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

static void registerSignals();

auto main(int argc, char *argv[]) -> int
{
  spdlog::cfg::load_env_levels();
  registerSignals();

  spdlog::info("Starting...");
  smv::init();

  std::atexit([] {
    smv::deinit();
    spdlog::info("Finished");
  });

  QCoreApplication::setOrganizationName("Ubiquity");
  QCoreApplication::setApplicationName("ShareMyView");
  QCoreApplication::setOrganizationDomain("labs.ecfreno.com");
  // https://doc.qt.io/qt-5/qtqml-syntax-propertybinding.html#debugging-overwriting-of-bindings
  QLoggingCategory::setFilterRules(
    QStringLiteral("qt.qml.binding.removal.info=true"));
  QApplication app(argc, argv);
  qmlRegisterUncreatableType<CaptureModeClass>(
    "smv.app.CaptureMode",
    1,
    0,
    "CaptureMode",
    "Not creatable as it is an enum");
  qmlRegisterUncreatableType<ScreenshotFormatClass>(
    "smv.app.ScreenshotFormat",
    1,
    0,
    "ScreenshotFormat",
    "Not creatable as it is an enum");
  qRegisterMetaType<CaptureMode>("CaptureMode");
  qRegisterMetaType<ScreenshotFormat>("ScreenshotFormat");

  const QUrl            mainUrl("qrc:/main.qml");
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
      // this error does not quit the program entirely,
      // so we need an else
      QCoreApplication::exit(EXIT_FAILURE);
    } else {
      QObject::disconnect(connection);
    }
  },
    Qt::QueuedConnection);

  engine.setOutputWarningsToStandardError(false);
  QObject::connect(&engine,
                   &QQmlApplicationEngine::warnings,
                   [](const QList<QQmlError> &warnings) {
    for (const auto &warning : warnings) {
      spdlog::error(warning.toString().toStdString());
    }
  });

  auto typeId = AppCore::registerInstance();
  // NOLINTBEGIN(cppcoreguidelines-owning-memory)
  engine.addImageProvider("smv", new AppImageProvider);
  // NOLINTEND(cppcoreguidelines-owning-memory)
  engine.load(mainUrl);
  if (auto *window = dynamic_cast<QWindow *>(engine.rootObjects().first())) {
    auto *appCore = engine.singletonInstance<AppCore *>(typeId);
    appCore->setQmlWindow(window);
  }
  return QGuiApplication::exec();
}

void sigHandler(int /*unused*/)
{
  static std::once_flag flag;
  std::call_once(flag, &QCoreApplication::quit);
}

void registerSignals()
{
  std::signal(SIGINT, sigHandler);
  std::signal(SIGTERM, sigHandler);
  std::signal(SIGQUIT, sigHandler);
}

namespace smv::log {
  extern std::shared_ptr<spdlog::logger> const logger =
    spdlog::stderr_color_mt(LOGGER_NAME_NATIVECLIENT);
}
