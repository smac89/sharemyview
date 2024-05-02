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

auto ScreenshotFormatClass::fromString(const QString &value) -> ScreenshotFormat
{
  return static_cast<Value>(metaEnum.keyToValue(value.toLocal8Bit()));
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

auto saveScreenshot(const QImage  &image,
                    const QString &location,
                    const QString &name) -> QString
{
  fs::create_directories(location.toStdString());

  const auto savePath = location + QDir::separator() + name;

  if (!image.save(savePath)) {
    // TODO: handle error
    spdlog::error("Failed to save screenshot: {}", savePath.toStdString());
  }
  return savePath;
}
