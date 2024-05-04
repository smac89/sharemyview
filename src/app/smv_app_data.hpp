#pragma once

#include "smv_utils.hpp"

#include <QCursor>
#include <QGuiApplication>
#include <QObject>
#include <QQmlEngine>
#include <QScreen>
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
  Q_PROPERTY(QScreen *cursorScreen READ cursorScreen)

  explicit AppData(QObject *parent = nullptr);

private:
  const QSharedPointer<ScreenshotsConfig> mScreenshot;

public:
  ~AppData() override = default;

  auto        screenshot() -> ScreenshotsConfig *;
  static auto create(QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject *;
  static constexpr auto QML_NAME = "AppData";
  static constexpr auto QML_URI  = "smv.app.AppData";
  // https://doc.qt.io/qt-5/qqmlengine.html#singletonInstance
  // https://forum.qt.io/topic/70370/how-should-i-get-the-qml-application-engine
  static const int   typeId;
  inline static auto cursorScreen() -> QScreen *
  {
    return QGuiApplication::screenAt(QCursor::pos());
  }
  inline static auto registerType()
  {
    auto typeId = qmlRegisterSingletonType<AppData>(
      QML_URI, 1, 0, QML_NAME, AppData::create);
    spdlog::info("AppData registered. Name={}, URL={}", QML_NAME, QML_URI);
    return typeId;
  }
};

class ScreenshotsConfig: public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString category READ category CONSTANT)
  Q_PROPERTY(QStringList formatsList READ formatsList CONSTANT)

  Q_PROPERTY(ScreenshotFormat formatEnum READ formatEnum STORED false)
  Q_PROPERTY(QString format MEMBER mFormat WRITE setFormat NOTIFY formatChanged)
  Q_PROPERTY(QString prefix MEMBER mPrefix WRITE setPrefix NOTIFY prefixChanged)
  Q_PROPERTY(QString suffix MEMBER mSuffix WRITE setSuffix NOTIFY suffixChanged)
  Q_PROPERTY(QString saveLocation MEMBER mSaveLocation WRITE setSaveLocation
               NOTIFY saveLocationChanged)
signals:
  void formatChanged(const QString &);
  void prefixChanged(const QString &);
  void suffixChanged(const QString &);
  void saveLocationChanged(const QString &);

public:
  explicit ScreenshotsConfig(QObject *parent = nullptr);
  void setFormat(const QString &format);
  void setPrefix(const QString &prefix);
  void setSuffix(const QString &suffix);
  void setSaveLocation(const QString &saveLocation);
  auto formatEnum() const -> ScreenshotFormat;

private:
  QString mSaveLocation, mFormat, mPrefix, mSuffix;

  inline auto static category() -> const char * { return "Screenshots"; }
  inline auto static formatsList() -> QStringList
  {
    return ScreenshotFormatClass::allFormats;
  }
  friend class AppData;
};
