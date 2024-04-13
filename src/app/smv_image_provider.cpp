#include "smv_image_provider.hpp"

#include <QPainter>
#include <QSvgRenderer>

static auto constexpr DEFAULT_IMAGE_WIDTH  = 48;
static auto constexpr DEFAULT_IMAGE_HEIGHT = 48;

AppImageProvider::AppImageProvider()
  : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage AppImageProvider::requestImage(const QString &id,
                                      QSize         *size,
                                      const QSize   &requestedSize)
{
  auto imageSource = QString(":/%1").arg(id);
  auto width       = requestedSize.width();
  auto height      = requestedSize.height();

  if (!requestedSize.isValid()) {
    width  = DEFAULT_IMAGE_WIDTH;
    height = DEFAULT_IMAGE_HEIGHT;
  }
  if (size) {
    *size = QSize(width, height);
  }
  QImage image(width, height, QImage::Format_ARGB32);
  image.fill(Qt::transparent);

  if (id.endsWith(".svg")) {
    QSvgRenderer renderer(imageSource);
    QPainter     painter(&image);
    renderer.render(&painter);
  } else {
    image.load(imageSource);
  }
  return image;
}
