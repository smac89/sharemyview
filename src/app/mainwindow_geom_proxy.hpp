#pragma once

#include <mutex>

#include <QObject>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QQuickWindow>
#include <QRect>

class MainWindowGeomAnim: public QObject
{
  Q_OBJECT
  Q_PROPERTY(
    QRect geometry READ geometry WRITE setGeometry NOTIFY geometryUpdated)
  Q_PROPERTY(
    QPoint position READ position WRITE setPosition NOTIFY positionUpdated)
  Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeUpdated)
public:
  MainWindowGeomAnim(QObject *parent = nullptr)
    : QObject(parent)
  {
    mGeomAnimation = new QPropertyAnimation(this);
    mGeomAnimation->setTargetObject(this);
  }

  void setGeometry(const QRect &rect) { mTargetWindow->setGeometry(rect); }
  void setPosition(const QPoint &pos) { mTargetWindow->setPosition(pos); }
  void setSize(const QSize &size)
  {
    mTargetWindow->setWidth(size.width());
    mTargetWindow->setHeight(size.height());
  }
  QRect  geometry() const { return mTargetWindow->geometry(); }
  QPoint position() const { return mTargetWindow->geometry().topLeft(); }
  QSize  size() const { return mTargetWindow->geometry().size(); }

  void animateProperty(const QRect &rect)
  {
    std::lock_guard lk(mMutex);
    mGeomAnimation->stop();
    mGeomAnimation->setPropertyName("geometry");
    mGeomAnimation->setStartValue(mTargetWindow->geometry());
    mGeomAnimation->setEndValue(rect);
    doAnimation();
  }

  void animateProperty(const QPoint &point)
  {
    std::lock_guard lk(mMutex);
    mGeomAnimation->stop();
    mGeomAnimation->setPropertyName("position");
    mGeomAnimation->setStartValue(mTargetWindow->geometry().topLeft());
    mGeomAnimation->setEndValue(point);
    doAnimation();
  }

  void animateProperty(const QSize &size)
  {
    std::lock_guard lk(mMutex);
    mGeomAnimation->stop();
    mGeomAnimation->setPropertyName("size");
    mGeomAnimation->setStartValue(mTargetWindow->geometry().size());
    mGeomAnimation->setEndValue(size);
    doAnimation();
  }

  void doAnimation()
  {
    mGeomAnimation->setDuration(DEFAULT_ANIMATION_DURATION);
    mGeomAnimation->setEasingCurve(QEasingCurve::Linear);
    mGeomAnimation->start();
  }

signals:
  void geometryUpdated(const QRect &);
  void positionUpdated(const QPoint &);
  void sizeUpdated(const QSize &);

public slots:
  void qquickWindowReady(QQuickWindow *window) { mTargetWindow = window; }

private:
  std::mutex            mMutex;
  QQuickWindow         *mTargetWindow              = nullptr;
  QPropertyAnimation   *mGeomAnimation             = nullptr;
  static constexpr auto DEFAULT_ANIMATION_DURATION = 300;
};
