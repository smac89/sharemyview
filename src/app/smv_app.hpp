#pragma once
#include "smv/events.hpp"
#include "smv_utils.hpp"

#include <memory>
#include <shared_mutex>

#include <QJSEngine>
#include <QObject>
#include <QPropertyAnimation>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QRect>
#include <QVariant>
#include <spdlog/spdlog.h>

class AppCore: public QObject
{
  // https://doc.qt.io/qt-5/qtqml-cppintegration-exposecppattributes.html
  Q_OBJECT
  Q_PROPERTY(Mode mode MEMBER mMode NOTIFY modeChanged)
  Q_PROPERTY(std::shared_ptr<smv::Window> targetWindow READ targetWindow WRITE
               setTargetWindow NOTIFY targetWindowChanged)
public:
  explicit AppCore(QObject *parent = nullptr);

  enum class Mode
  {
    Region,
    Window
  };
  Q_ENUM(Mode)

  auto mode() const -> Mode;
  auto targetWindow() const -> std::shared_ptr<smv::Window>;
  void setTargetWindow(const std::shared_ptr<smv::Window> &);
  void operator()(const smv::EventDataMouseEnter &data);

  ~AppCore() override;

signals:
  void modeChanged(Mode);
  void targetWindowMoved(const QPoint &);
  void targetWindowResized(const QSize &);
  void targetWindowChanged(const QSize &, const QPoint &);
  void mediaCaptureStarted(CaptureMode);
  void mediaCaptureSuccess(CaptureMode, const QVariant &);
  void mediaCaptureFailed(CaptureMode, const QString &);
  void mediaCaptureStopped(CaptureMode);

private slots:
  void startRecording();
  void streamRecording();

public slots:
  void updateRecordRegion(const QRect &rect);
  void updateRecordRegion(const QPoint &);
  void updateRecordRegion(const QSize &);
  void updateRecordRegion(const QSize &, const QPoint &);
  void takeScreenshot(const QRect &rect, QObject *);

private:
  QRect                        mRecordRegion;
  Mode                         mMode = Mode::Window;
  smv::Cancel                  mCancel;
  std::shared_mutex            mMutex;
  std::weak_ptr<smv::Window>   mActiveWindow;
  std::shared_ptr<smv::Window> mTargetWindow;

public:
  static constexpr auto               QML_NAME = "AppCore";
  [[maybe_unused]] inline static auto registerInstance() -> int
  {
    static AppCore appCore;
    // register the app core as a singleton. see:
    // https://doc.qt.io/qt-5/qtqml-cppintegration-overview.html#choosing-the-correct-integration-method-between-c-and-qml
    auto typeId =
      qmlRegisterSingletonInstance("smv.app.AppCore", 1, 0, QML_NAME, &appCore);
    spdlog::info(
      "AppCore registered. Name={}, URL={}", QML_NAME, "smv.app.AppCore");
    return typeId;
  }
};
