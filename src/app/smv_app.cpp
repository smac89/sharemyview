#include "smv_app.hpp"
#include "smv/utils/autocancel.hpp"
#include "smv/winclient.hpp"
#include "smv/window.hpp"

#include <QPropertyAnimation>
#include <spdlog/spdlog.h>

// QT Globals: https://doc.qt.io/qt-5/qtglobal.html

App::App(QObject *parent)
  : QObject(parent)
  , mGeomAnimation(this)
  , mCancel(smv::listen<smv::EventType::MouseEnter, smv::EventDataMouseEnter>(
      std::bind(&App::operator(), this, std::placeholders::_1)))
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
      if (std::shared_lock lk(mMutex); this->mTargetWindow == nullptr) {
        return;
      }
      this->setTargetWindow(nullptr);
      spdlog::info("Tracking region", data.x, data.y);
    } else {
      if (std::shared_lock lk(mMutex); window == this->mTargetWindow) {
        return;
      }
      this->setTargetWindow(window);
      spdlog::info("Mouse enter {{x: {}, y: {}}}", data.x, data.y);
    }
  }
}

void App::takeScreenShot()
{
  // fbo->toImage().save("capture.png");
  spdlog::info("Taking screenshot...");
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

void App::qquickWindowReady(QQuickWindow *window)
{
  mRecordRegion = window->geometry();
  mGeomAnimation.qquickWindowReady(window);
}

App::Mode App::mode() const
{
  return mMode;
}

std::shared_ptr<smv::Window> App::targetWindow() const
{
  return mTargetWindow;
}

void App::setTargetWindow(const std::shared_ptr<smv::Window> window)
{
  using AutoCancel                      = smv::utils::AutoCancel;
  static smv::Cancel cancelWindowMove   = nullptr;
  static smv::Cancel cancelWindowResize = nullptr;

  if (window != nullptr) {
    spdlog::info("Tracking window {:#x}", window->id());
    {
      std::unique_lock lk(mMutex);
      this->mTargetWindow = window;
    }

    cancelWindowMove = AutoCancel::wrap(smv::listen<smv::EventType::WindowMove>(
      window->id(), [this](const smv::EventDataWindowMove &data) {
      emit targetWindowMoved(QPoint(data.x, data.y));
    }));

    cancelWindowResize =
      AutoCancel::wrap(smv::listen<smv::EventType::WindowResize>(
        window->id(), [this](const smv::EventDataWindowResize &data) {
      emit targetWindowResized(QSize(data.w, data.h));
    }));

    emit targetWindowChanged(
      QSize(window->size().w, window->size().h),
      QPoint(window->position().x, window->position().y));

  } else {
    cancelWindowMove   = nullptr;
    cancelWindowResize = nullptr;
    std::unique_lock lk(mMutex);
    this->mTargetWindow = window;
  }
}

App::~App()
{
  mCancel();
  spdlog::info("App destroyed");
}

// void
// ShareMyViewWindow::paintEvent(QPaintEvent *)
// {
//   QPainter painter(this);
//   QPainterPath path;
//   QSizeF itemSize = size();
//   path.addRoundedRect(0, 0, itemSize.width(), itemSize.height(), 10, 10);
//   painter.setRenderHint(QPainter::Antialiasing);
//   painter.strokePath(path, QPen(Qt::gray, 10));
// }
