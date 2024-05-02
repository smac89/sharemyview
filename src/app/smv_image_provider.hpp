#pragma once

#include <QImage>
#include <QObject>
#include <QQuickImageProvider>

// TODO: Consider using QQuickAsyncImageProvider.
// https://doc.qt.io/qt-5/qquickasyncimageprovider.html
class AppImageProvider
  : public QObject
  , public QQuickImageProvider
{
public:
  explicit AppImageProvider(QObject * = nullptr);
  auto requestImage(const QString & /*id*/,
                    QSize * /*size*/,
                    const QSize & /*requestedSize*/) -> QImage override;
};
