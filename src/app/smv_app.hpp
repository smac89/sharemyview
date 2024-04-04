#pragma once
#include <QObject>
#include <QQuickPaintedItem>

class ShareMyViewWidget : public QQuickPaintedItem
{
  Q_OBJECT
  QML_ELEMENT
public:
  ShareMyViewWidget(QQuickItem *parent = nullptr);
  void paint(QPainter *painter);
  // virtual ~ShareMyViewWidget() {}

public slots:
  void captureWindow();
};
