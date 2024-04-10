#include "smv_app.hpp"
#include "smv/winclient.hpp"

#include <exception>

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQuickPaintedItem>
#include <QScreen>
#include <QStackedLayout>
#include <QWindow>
#include <cstdint>
#include <spdlog/spdlog.h>

ShareMyViewWindow::ShareMyViewWindow(QWindow *parent)
  : QQuickWindow(parent)
  , currentPos(QPoint(0, 0))
  , mCancel(smv::listen<smv::EventDataMouseEnter>(
      [](const smv::EventDataMouseEnter &data) {
  // const auto &enterEvent =
  //   dynamic_cast<const smv::EventDataMouseEnter &>(data);
  spdlog::info("Mouse enter {{x: {}, y: {}}}", data.x, data.y);
}))
{
  installEventFilter(this);
}

ShareMyViewWindow::~ShareMyViewWindow()
{
  mCancel();
}

bool ShareMyViewWindow::eventFilter(QObject *, QEvent *event)
{
  if (event->type() == QEvent::MouseButtonPress) {
    currentPos              = QCursor::pos();
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->buttons() == Qt::LeftButton) {
      // TODO: take screenshot here
      spdlog::info(
        "Mouse click at {{x: {}, y: {}}}", mouseEvent->x(), mouseEvent->y());
    }
  }
  return false;
}

void ShareMyViewWindow::takeScreenShot()
{
  // fbo->toImage().save("capture.png");
}

void ShareMyViewWindow::startRecording() {}

void ShareMyViewWindow::streamRecording() {}

void ShareMyViewWindow::setMode(const Mode mode)
{
  mMode = mode;
  emit modeChanged(mode);
}

ShareMyViewWindow::Mode ShareMyViewWindow::mode() const
{
  return mMode;
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
