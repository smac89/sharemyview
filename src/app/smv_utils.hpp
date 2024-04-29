#pragma once

#include "smv/record.hpp"

#include <type_traits>

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
Q_DECLARE_METATYPE(CaptureMode);
// qRegisterMetaType<CaptureMode>("CaptureMode");

class ScreenshotFormatClass
{
  Q_GADGET
  using FormatType = std::underlying_type_t<smv::ScreenshotFormat>;

public:
  enum class Value
  {
    PPM = static_cast<FormatType>(smv::ScreenshotFormat::PPM),
    PNG = static_cast<FormatType>(smv::ScreenshotFormat::PNG),
    JPG = static_cast<FormatType>(smv::ScreenshotFormat::JPG),
  };
  Q_ENUM(Value);

private:
  explicit ScreenshotFormatClass() = default;

public:
  static auto toString(Value value) -> QString;
};
using ScreenshotFormat = ScreenshotFormatClass::Value;
Q_DECLARE_METATYPE(ScreenshotFormat);
// qRegisterMetaType<ScreenshotFormat>("ScreenshotFormat");

auto saveScreenshot(const QImage &image, const QString &name) -> QString;
