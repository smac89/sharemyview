#pragma once

#include "smv/record.hpp"

#include <type_traits>

#include <QMetaEnum>
#include <QObject>

/**
 * @brief Converts a QRect to a smv::Region
 *
 * @param rect
 * @return smv::Region
 */
auto rectToRegion(const QRect &rect) -> smv::Region;

// https://qml.guide/enums-in-qt-qml/
class CaptureModeClass
{
  Q_GADGET
public:
  enum class Value
  {
    Screenshot = 0x1,
    Record     = 0x2,
    Stream     = 0x4
  };
  Q_ENUM(Value)

private:
  explicit CaptureModeClass() = default;
};
using CaptureMode = CaptureModeClass::Value;
// Q_DECLARE_METATYPE(CaptureMode);
// qRegisterMetaType<CaptureMode>("CaptureMode");

class ScreenshotFormatClass
{
  Q_GADGET
  using FormatType = std::underlying_type_t<smv::ScreenshotFormat>;

public:
  enum class Value
  {
    PNG = static_cast<FormatType>(smv::ScreenshotFormat::PNG),
    JPG = static_cast<FormatType>(smv::ScreenshotFormat::JPG),
    PPM = static_cast<FormatType>(smv::ScreenshotFormat::PPM),
  };
  Q_ENUM(Value);

private:
  explicit ScreenshotFormatClass() = default;
  static const QMetaEnum metaEnum;

public:
  static auto              formatToString(Value value) -> QString;
  static const QStringList allFormats;
};
using ScreenshotFormat = ScreenshotFormatClass::Value;
// Q_DECLARE_METATYPE(ScreenshotFormat);
// qRegisterMetaType<ScreenshotFormat>("ScreenshotFormat");

/**
 * @brief Save the screenshot to a file at an appropriate location
 *
 * @param image The image to save
 * @param name The name used to save the image
 * @return QString
 */
auto saveScreenshot(const QImage &image, const QString &name) -> QString;
