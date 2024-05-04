#include "smv_app.hpp"
#include "smv/common/autocancel.hpp"
#include "smv/events.hpp"
#include "smv/record.hpp"
#include "smv/winclient.hpp"
#include "smv/window.hpp"
#include "smv_capture.hpp"
#include "smv_utils.hpp"

#include <QDateTime>
#include <QMetaEnum>
#include <QObject>
#include <QPropertyAnimation>
#include <QQmlApplicationEngine>
#include <QStandardPaths>
#include <QThread>
#include <spdlog/spdlog.h>

// QT Globals: https://doc.qt.io/qt-5/qtglobal.html
AppCore::AppCore(QObject *parent)
  : QObject(parent)
  , mCancel(smv::listen<smv::EventType::MouseEnter>(
      [this](const smv::EventDataMouseEnter &data) {
  (*this)(data);
}))
{
  QObject::connect(this,
                   &AppCore::targetWindowMoved,
                   this,
                   qOverload<const QPoint &>(&AppCore::updateRecordRegion));
  QObject::connect(this,
                   &AppCore::targetWindowResized,
                   this,
                   qOverload<const QSize &>(&AppCore::updateRecordRegion));
  QObject::connect(
    this,
    &AppCore::targetWindowChanged,
    this,
    qOverload<const QSize &, const QPoint &>(&AppCore::updateRecordRegion));
  // connect all media capture events
  QObject::connect(
    this, &AppCore::mediaCaptureSuccess, this, [this](CaptureMode mode) {
    emit mediaCaptureStopped(mode);
  });
  QObject::connect(
    this, &AppCore::mediaCaptureFailed, this, [this](CaptureMode mode) {
    emit mediaCaptureStopped(mode);
  });
}

void AppCore::operator()(const smv::EventDataMouseEnter &data)
{
  if (auto window = data.window.lock(); window != nullptr) {
    if (mMode == Mode::Region) {
      if (std::shared_lock _(mMutex); this->mTargetWindow == nullptr) {
        return;
      }
      this->setTargetWindow(nullptr);
      spdlog::info("Tracking region", data.x, data.y);
    } else {
      if (std::shared_lock _(mMutex); window == this->mTargetWindow) {
        return;
      }
      this->setTargetWindow(window);
      spdlog::info("Mouse enter {{x: {}, y: {}}}", data.x, data.y);
    }
  }
}

void AppCore::setQmlWindow(QWindow *window)
{
  mQmlWindow = window;
}

void AppCore::takeScreenshot(const QRect &rect, QObject *screenshotConfig)
{
  emit mediaCaptureStarted(CaptureMode::Screenshot);
  spdlog::info("Taking screenshot: x: {}, y: {}, w: {}, h: {}",
               rect.x(),
               rect.y(),
               rect.width(),
               rect.height());
  auto config = smv::ScreenshotConfig { rectToRegion(rect) };
  if (!config.isValid()) {
    auto msg =
      fmt::format("Invalid screenshot config: x: {}, y: {}, w: {}, h: {}",
                  rect.x(),
                  rect.y(),
                  rect.width(),
                  rect.height());
    emit mediaCaptureFailed(CaptureMode::Screenshot,
                            QString::fromStdString(msg));
    spdlog::error(msg);
    return;
  }
  auto format     = screenshotConfig->property("format").toString();
  auto formatEnum = ScreenshotFormatClass::fromString(format);
  // TODO check if format is valid

  smv::capture(config,
               static_cast<smv::ScreenshotFormat>(formatEnum),
               [this, format, &screenshotConfig](smv::CaptureSource &source) {
    if (auto err = source.error()) {
      auto msg = fmt::format("Failed to capture screenshot: {}", err.value());
      emit mediaCaptureFailed(CaptureMode::Screenshot,
                              QString::fromStdString(msg));
      spdlog::error(msg);
      return;
    }
    const auto &extension = format;
    auto        imageIO   = CaptureSourceIO(&source);
    auto        fileName  = QString("%1%2.%3")
                      .arg(screenshotConfig->property("prefix").toString())
                      .arg(QDateTime::currentDateTime().toString(
                        screenshotConfig->property("suffix").toString()))
                      .arg(extension.toLower());

    emit mediaCaptureSuccess(
      CaptureMode::Screenshot,
      saveScreenshot(imageIO,
                     screenshotConfig->property("saveLocation").toString(),
                     fileName));
  });
}

void AppCore::startRecording() {}

void AppCore::streamRecording() {}

void AppCore::updateRecordRegion(const QRect &rect)
{
  mRecordRegion = rect;
}

void AppCore::updateRecordRegion(const QPoint &point)
{
  mRecordRegion.moveTo(point);
}

void AppCore::updateRecordRegion(const QSize &size)
{
  mRecordRegion.setSize(size);
}

void AppCore::updateRecordRegion(const QSize &size, const QPoint &point)
{
  mRecordRegion = QRect(point, size);
}

auto AppCore::targetWindow() const -> std::shared_ptr<smv::Window>
{
  return mTargetWindow;
}

void AppCore::setTargetWindow(const std::shared_ptr<smv::Window> &window)
{
  using AutoCancel                      = smv::utils::AutoCancel;
  static smv::Cancel cancelWindowMove   = nullptr;
  static smv::Cancel cancelWindowResize = nullptr;

  if (window != nullptr) {
    spdlog::info("Tracking window {:#x}", window->id());
    {
      // unique_lock is used to actually lock the mutex, rather than sharing
      std::unique_lock _(mMutex);
      this->mTargetWindow = window;
    }

    cancelWindowMove = AutoCancel::wrap(smv::listen<smv::EventType::WindowMove>(
      window->id(), [this](const smv::EventDataWindowMove &data) {
      emit targetWindowMoved(QPoint(data.x, data.y));
    }));

    cancelWindowResize =
      AutoCancel::wrap(smv::listen<smv::EventType::WindowResize>(
        window->id(), [this](const smv::EventDataWindowResize &data) {
      emit targetWindowResized(
        QSize(static_cast<int>(data.w), static_cast<int>(data.h)));
    }));

    emit targetWindowChanged(
      QSize(static_cast<int>(window->size().w),
            static_cast<int>(window->size().h)),
      QPoint(window->position().x, window->position().y));

  } else {
    cancelWindowMove   = nullptr;
    cancelWindowResize = nullptr;
    // unique_lock is used to actually lock the mutex, rather than sharing
    std::unique_lock _(mMutex);
    this->mTargetWindow = window;
  }
}

AppCore::~AppCore()
{
  mCancel();
  spdlog::info("App destroyed");
}
