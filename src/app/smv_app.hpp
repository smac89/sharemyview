#pragma once
#include "mainwindow_geom_proxy.hpp"
#include "smv/events.hpp"
#include "smv_capture.hpp"

#include <QObject>
#include <QPropertyAnimation>
#include <QQuickWindow>
#include <QRect>
#include <memory>
#include <shared_mutex>

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

class App: public QObject
{
  // https://doc.qt.io/qt-5/qtqml-cppintegration-exposecppattributes.html
  Q_OBJECT
  Q_PROPERTY(Mode mode MEMBER mMode NOTIFY modeChanged)
  Q_PROPERTY(std::shared_ptr<smv::Window> targetWindow READ targetWindow WRITE
               setTargetWindow NOTIFY targetWindowChanged)
public:
  explicit App(QObject *parent = nullptr);

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

  ~App() override;

signals:
  void modeChanged(Mode);
  void targetWindowMoved(const QPoint &);
  void targetWindowResized(const QSize &);
  void targetWindowChanged(const QSize &, const QPoint &);
  void mediaCaptureStarted(CaptureMode);
  void mediaCaptureStopped(CaptureMode);

public slots:
  void updateRecordRegion(const QRect &rect);
  void updateRecordRegion(const QPoint &);
  void updateRecordRegion(const QSize &);
  void updateRecordRegion(const QSize &, const QPoint &);
  void takeScreenshot(const QRect &rect);
  // void qquickWindowReady(QQuickWindow *window);

private slots:
  void startRecording();
  void streamRecording();

private:
  QRect                        mRecordRegion;
  MainWindowGeomAnim           mGeomAnimation;
  Mode                         mMode = Mode::Window;
  smv::Cancel                  mCancel;
  std::shared_mutex            mMutex;
  std::weak_ptr<smv::Window>   mActiveWindow;
  std::shared_ptr<smv::Window> mTargetWindow;
};
