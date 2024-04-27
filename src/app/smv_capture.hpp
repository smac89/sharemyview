#pragma once

#include <QObject>

// https://qml.guide/enums-in-qt-qml/
class CaptureModeClass: public QObject
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
