#pragma once

#include <QImage>
#include <QQuickImageProvider>

class AppImageProvider: public QQuickImageProvider
{
public:
  AppImageProvider();
  QImage requestImage(const QString &id,
                      QSize         *size,
                      const QSize   &requestedSize) override;
};
