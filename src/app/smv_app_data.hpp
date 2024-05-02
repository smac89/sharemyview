#pragma once

#include "smv_utils.hpp"

#include <QObject>
#include <QQmlEngine>
#include <QSharedMemory>
#include <QString>
#include <QStringList>
#include <spdlog/spdlog.h>

class ScreenshotsConfig;

/**
 * @brief Store the default settings for the application
 */
class AppData: public QObject
{
  Q_OBJECT
  Q_PROPERTY(ScreenshotsConfig *screenshot READ screenshot CONSTANT)

  explicit AppData(QObject *parent = nullptr);

public:
  ~AppData() override = default;

  static auto create(QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject *;
  auto        screenshot() -> ScreenshotsConfig        *{ return mScreenshot.data(); }

private:
  const QSharedPointer<ScreenshotsConfig> mScreenshot;

public:
  static constexpr auto QML_NAME = "AppData";
  static const int      typeId;
  inline static auto    registerType()
  {
    auto typeId = qmlRegisterSingletonType<AppData>(
      "smv.app.AppData", 1, 0, QML_NAME, AppData::create);
    spdlog::info(
      "AppData registered. Name={}, URL={}", QML_NAME, "smv.app.AppData");
    return typeId;
  }
};

class ScreenshotsConfig: public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString category READ category CONSTANT)
  Q_PROPERTY(QStringList formatsList READ formatsList CONSTANT)

  Q_PROPERTY(QString format MEMBER mFormat CONSTANT)
  Q_PROPERTY(QString prefix MEMBER mPrefix CONSTANT)
  Q_PROPERTY(QString suffix MEMBER mSuffix CONSTANT)
  Q_PROPERTY(QString folder MEMBER mFolder CONSTANT)

public:
  explicit ScreenshotsConfig(QObject *parent = nullptr);
  inline auto static formatsList() -> QStringList
  {
    return ScreenshotFormatClass::allFormats;
  }

private:
  QString mFolder;
  QString mFormat;
  QString mPrefix;
  QString mSuffix;

  inline auto static category() -> const char * { return "Screenshots"; }
  friend class AppData;
};
