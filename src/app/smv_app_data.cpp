#include "smv_app_data.hpp"
#include "qobject.h"

#include <filesystem>
#include <regex>

#include <QCoreApplication>
#include <QDir>
#include <QMetaEnum>
#include <QQmlEngine>
#include <QSettings>
#include <QSharedPointer>
#include <QStandardPaths>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

AppData::AppData(QObject *parent)
  : QObject(parent)
  , mScreenshot(QSharedPointer<ScreenshotsConfig>::create())
{
  // qRegisterMetaType<QSharedPointer<ScreenshotsConfig>>();
  spdlog::info("AppData created");
}

ScreenshotsConfig::ScreenshotsConfig(QObject *parent)
  : QObject(parent)
{
  QSettings settings;
  settings.beginGroup(category());
  if (auto folder = settings.value("folder"); folder.isNull()) {
    std::regex pat(u8R"(\bscreenshots?\b)",
                   std::regex_constants::ECMAScript |
                     std::regex_constants::icase);

    auto picturesFolder  = fs::path { QStandardPaths::writableLocation(
                                       QStandardPaths::PicturesLocation)
                                       .toStdString() };
    auto screenshotsPath = picturesFolder / "Screenshots";
    if (fs::exists(picturesFolder)) {
      // check if screenshot(s) folder already exists
      for (const auto &entry : fs::directory_iterator(picturesFolder)) {
        if (std::regex_match(entry.path().filename().u8string(), pat)) {
          screenshotsPath = entry.path();
          break;
        }
      }
    }
    auto appScreenshots =
      screenshotsPath /
      QCoreApplication::applicationName().toLower().toStdString();
    mFolder = QString::fromStdString(appScreenshots.string());
    settings.setValue("folder", mFolder);
  } else {
    mFolder = folder.toString();
  }

  if (auto format = settings.value("format"); format.isNull()) {
    if (!formatsList().isEmpty()) {
      mFormat = formatsList().first();
      settings.setValue("format", mFormat);
    }
  } else {
    mFormat = format.toString();
  }

  if (auto prefix = settings.value("prefix"); prefix.isNull()) {
    mPrefix = "Screenshot_";
    settings.setValue("prefix", mPrefix);
  } else {
    mPrefix = prefix.toString();
  }

  if (auto suffix = settings.value("suffix"); suffix.isNull()) {
    mSuffix = "yyyy-MMM-dd_hh-mm-ss-zzz";
    settings.setValue("suffix", mSuffix);
  } else {
    mSuffix = suffix.toString();
  }
}

auto AppData::create([[maybe_unused]] QQmlEngine *engine,
                     [[maybe_unused]] QJSEngine  *scriptEngine) -> QObject *
{
  return new AppData;
}

const int AppData::typeId = AppData::registerType();
