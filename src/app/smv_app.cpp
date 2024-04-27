#include "smv_app.hpp"
#include "qimage.h"
#include "qstringview.h"
#include "smv/common/autocancel.hpp"
#include "smv/events.hpp"
#include "smv/record.hpp"
#include "smv/winclient.hpp"
#include "smv/window.hpp"
#include "smv_capture.hpp"
#include "smv_utils.hpp"

#include <QBuffer>
#include <QPropertyAnimation>
#include <spdlog/spdlog.h>

// QT Globals: https://doc.qt.io/qt-5/qtglobal.html
App::App(QObject *parent)
  : QObject(parent)
  , mGeomAnimation(this)
  , mCancel(smv::listen<smv::EventType::MouseEnter>(
      [this](const smv::EventDataMouseEnter &data) {
  (*this)(data);
}))
{
  QObject::connect(this,
                   &App::targetWindowMoved,
                   this,
                   qOverload<const QPoint &>(&App::updateRecordRegion));
  QObject::connect(this,
                   &App::targetWindowResized,
                   this,
                   qOverload<const QSize &>(&App::updateRecordRegion));
  QObject::connect(
    this,
    &App::targetWindowChanged,
    this,
    qOverload<const QSize &, const QPoint &>(&App::updateRecordRegion));
}

void App::operator()(const smv::EventDataMouseEnter &data)
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

void App::takeScreenshot(const QRect &rect)
{
  spdlog::info("Taking screenshot: x: {}, y: {}, w: {}, h: {}",
               rect.x(),
               rect.y(),
               rect.width(),
               rect.height());
  auto config = smv::ScreenshotConfig { rectToRegion(rect) };
  if (!config.isValid()) {
    spdlog::error("Invalid screenshot config");
    return;
  }
  auto format = smv::ScreenshotFormat::PNG;
  emit mediaCaptureStarted(CaptureMode::Screenshot);
  smv::capture(config, format, [this](smv::CaptureSource &source) {
    if (auto err = source.error()) {
      spdlog::error("Failed to capture screenshot: {}", err.value());
      return;
    }
    spdlog::info("Screenshot captured");
    auto   imageIO = CaptureSourceIO(&source);
    QImage image;
    if (!image.load(&imageIO, "png")) {
      spdlog::error("Failed to parse image as PNG");
      return;
    }
    image.save("/tmp/screenshot.png");
    emit mediaCaptureStopped(CaptureMode::Screenshot);
  });
}

void App::startRecording() {}

void App::streamRecording() {}

void App::updateRecordRegion(const QRect &rect)
{
  mRecordRegion = rect;
}

void App::updateRecordRegion(const QPoint &point)
{
  mRecordRegion.moveTo(point);
}

void App::updateRecordRegion(const QSize &size)
{
  mRecordRegion.setSize(size);
}

void App::updateRecordRegion(const QSize &size, const QPoint &point)
{
  mRecordRegion = QRect(point, size);
}

// void App::qquickWindowReady(QQuickWindow *window)
// {
//   mRecordRegion = window->geometry();
//   mGeomAnimation.qquickWindowReady(window);
// }

auto App::mode() const -> App::Mode
{
  return mMode;
}

auto App::targetWindow() const -> std::shared_ptr<smv::Window>
{
  return mTargetWindow;
}

void App::setTargetWindow(const std::shared_ptr<smv::Window> &window)
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

App::~App()
{
  mCancel();
  spdlog::info("App destroyed");
}
