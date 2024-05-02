#include "smv_image_provider.hpp"

#include <QPainter>
#include <QSvgRenderer>

static auto constexpr DEFAULT_IMAGE_WIDTH  = 48;
static auto constexpr DEFAULT_IMAGE_HEIGHT = 48;

AppImageProvider::AppImageProvider(QObject *parent)
  : QObject(parent)
  , QQuickImageProvider(QQuickImageProvider::Image)
{
}

// TODO: Apparently svg images can be slow to load.
// Can we make use of some of the tips given here:
// https://wiki.qt.io/Performance_tip_Images, to make it faster?
auto AppImageProvider::requestImage(const QString &imgId,
                                    QSize         *size,
                                    const QSize   &requestedSize) -> QImage
{
  auto imageSource = QString(":/%1").arg(imgId);
  auto width       = requestedSize.width();
  auto height      = requestedSize.height();

  if (!requestedSize.isValid()) {
    width  = DEFAULT_IMAGE_WIDTH;
    height = DEFAULT_IMAGE_HEIGHT;
  }
  if (size != nullptr) {
    *size = QSize(width, height);
  }
  QImage image(width, height, QImage::Format_ARGB32);
  image.fill(Qt::transparent);

  if (imgId.endsWith(".svg", Qt::CaseInsensitive)) {
    QSvgRenderer renderer(imageSource);
    QPainter     painter(&image);
    renderer.render(&painter);
  } else {
    image.load(imageSource);
  }
  return image;
}
