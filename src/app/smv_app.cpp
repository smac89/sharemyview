#include "smv_app.hpp"
#include "smv/winclient.hpp"
#include <QQuickPaintedItem>
#include <spdlog/spdlog.h>

ShareMyViewWidget::ShareMyViewWidget(QQuickItem *parent)
  : QQuickPaintedItem(parent)
{
  // initialize the widget
}

void
ShareMyViewWidget::captureWindow()
{
  // capture the window
}

void
ShareMyViewWidget::paint(QPainter *painter)
{
  // paint the window
}
