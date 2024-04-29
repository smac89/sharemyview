#include "smv_utils.hpp"
#include "qcoreapplication.h"
#include "smv/window.hpp"

#include <filesystem>

#include <QDir>
#include <QImage>
#include <QMetaEnum>
#include <QRect>
#include <QStandardPaths>

namespace fs = std::filesystem;

auto rectToRegion(const QRect &rect) -> smv::Region
{
  return { static_cast<uint32_t>(rect.width()),
           static_cast<uint32_t>(rect.height()),
           rect.x(),
           rect.y() };
}

auto ScreenshotFormatClass::toString(Value value) -> QString
{
  static auto metaEnum = QMetaEnum::fromType<ScreenshotFormat>();
  return metaEnum.valueToKey(static_cast<int>(value));
}

auto saveScreenshot(const QImage &image, const QString &name) -> QString
{
  auto picturesFolder  = fs::path { QStandardPaths::writableLocation(
                                     QStandardPaths::PicturesLocation)
                                     .toStdString() };
  auto screenshotsPath = picturesFolder / "Screenshots";

  if (!fs::exists(picturesFolder)) {
    fs::create_directories(screenshotsPath);
  } else {
    for (const auto &entry : fs::directory_iterator(picturesFolder)) {
      if (QString::compare(entry.path().filename().c_str(),
                           screenshotsPath.filename().c_str(),
                           Qt::CaseInsensitive) == 0) {
        screenshotsPath = entry.path();
        break;
      }
    }
  }

  auto appScreenshots =
    screenshotsPath / QCoreApplication::applicationName().toStdString();

  fs::create_directories(appScreenshots);

  const auto finalSavePath =
    QString::fromStdString(appScreenshots.string()) + QDir::separator() + name;

  image.save(finalSavePath);
  return finalSavePath;
}
