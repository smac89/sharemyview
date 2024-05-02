#include "smv_utils.hpp"
#include "qobjectdefs.h"
#include "smv/window.hpp"

#include <filesystem>
#include <regex>

#include <QCoreApplication>
#include <QDir>
#include <QImage>
#include <QMetaEnum>
#include <QRect>
#include <QStandardPaths>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

auto rectToRegion(const QRect &rect) -> smv::Region
{
  return { static_cast<uint32_t>(rect.width()),
           static_cast<uint32_t>(rect.height()),
           rect.x(),
           rect.y() };
}

auto ScreenshotFormatClass::formatToString(Value value) -> QString
{
  return metaEnum.valueToKey(static_cast<int>(value));
}

const QMetaEnum ScreenshotFormatClass::metaEnum =
  QMetaEnum::fromType<ScreenshotFormat>();

const QStringList ScreenshotFormatClass::allFormats = []() {
  QStringList formats;
  for (int i = 0; i < metaEnum.keyCount(); ++i) {
    formats << formatToString(static_cast<ScreenshotFormat>(metaEnum.value(i)));
  }
  return formats;
}();

auto saveScreenshot(const QImage &image, const QString &name) -> QString
{

  const static std::regex screenshots(u8R"(\bScreenshots?\b)",
                                      std::regex_constants::ECMAScript |
                                        std::regex_constants::icase);

  auto picturesFolder  = fs::path { QStandardPaths::writableLocation(
                                     QStandardPaths::PicturesLocation)
                                     .toStdString() };
  auto screenshotsPath = picturesFolder / "Screenshots";

  if (!fs::exists(picturesFolder)) {
    fs::create_directories(screenshotsPath);
  } else {
    // check if screenshot(s) folder already exists
    for (const auto &entry : fs::directory_iterator(picturesFolder)) {
      if (std::regex_match(entry.path().filename().u8string(), screenshots)) {
        screenshotsPath = entry.path();
        break;
      }
    }
  }

  auto appScreenshots =
    screenshotsPath /
    QCoreApplication::applicationName().toLower().toStdString();

  fs::create_directories(appScreenshots);

  const auto finalSavePath =
    QString::fromStdString(appScreenshots.string()) + QDir::separator() + name;

  if (!image.save(finalSavePath)) {
    // TODO: handle error
    spdlog::error("Failed to save screenshot: {}", finalSavePath.toStdString());
  }
  return finalSavePath;
}
