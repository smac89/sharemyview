#include "smv_app_data.hpp"
#include "smv_utils.hpp"

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

auto AppData::screenshot() -> ScreenshotsConfig *
{
  return mScreenshot.data();
}

ScreenshotsConfig::ScreenshotsConfig(QObject *parent)
  : QObject(parent)
{
  QSettings settings;
  settings.beginGroup(category());
  if (auto saveLocation = settings.value("saveLocation");
      saveLocation.isNull()) {
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
    mSaveLocation = QString::fromStdString(appScreenshots.string());
    settings.setValue("saveLocation", mSaveLocation);
  } else {
    mSaveLocation = saveLocation.toString();
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

void ScreenshotsConfig::setFormat(const QString &format)
{
  if (mFormat != format) {
    mFormat = format;
    QSettings settings;
    settings.beginGroup(category());
    settings.setValue("format", mFormat);
    settings.endGroup();
    settings.sync();
  }
}

void ScreenshotsConfig::setPrefix(const QString &prefix)
{
  if (mPrefix != prefix) {
    mPrefix = prefix;
    QSettings settings;
    settings.beginGroup(category());
    settings.setValue("prefix", mPrefix);
    settings.endGroup();
    settings.sync();
  }
}

void ScreenshotsConfig::setSuffix(const QString &suffix)
{
  if (mSuffix != suffix) {
    mSuffix = suffix;
    QSettings settings;
    settings.beginGroup(category());
    settings.setValue("suffix", mSuffix);
    settings.endGroup();
    settings.sync();
  }
}

void ScreenshotsConfig::setSaveLocation(const QString &saveLocation)
{
  if (mSaveLocation != saveLocation) {
    mSaveLocation = saveLocation;
    QSettings settings;
    settings.beginGroup(category());
    settings.setValue("saveLocation", mSaveLocation);
    settings.endGroup();
    settings.sync();
  }
}

auto ScreenshotsConfig::formatEnum() const -> ScreenshotFormat
{
  return ScreenshotFormatClass::fromString(mFormat);
}

auto AppData::create([[maybe_unused]] QQmlEngine *engine,
                     [[maybe_unused]] QJSEngine  *scriptEngine) -> QObject *
{
  // NOLINTBEGIN(cppcoreguidelines-owning-memory)
  return new AppData;
  // NOLINTEND(cppcoreguidelines-owning-memory)
}

const int AppData::typeId = AppData::registerType();
