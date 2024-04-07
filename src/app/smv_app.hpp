#pragma once
#include <QObject>
#include <QOpenGLFramebufferObject>
#include <QQuickPaintedItem>
#include <QQuickWindow>
#include <QScopedPointer>
#include <QWindow>
#include <memory>

class ShareMyViewWindow : public QQuickWindow
{
  // https://doc.qt.io/qt-5/qtqml-cppintegration-exposecppattributes.html
  Q_OBJECT
  Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
public:
  explicit ShareMyViewWindow(QWindow *parent = nullptr);

  enum class Mode
  {
    None,
    Region,
    Window
  };
  Q_ENUM(Mode)

  void setMode(const Mode);
  Mode mode() const;

signals:
  void modeChanged(const Mode);

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void takeScreenShot();
  void startRecording();
  void streamRecording();

private:
  QPoint currentPos;
  Mode mMode = Mode::None;
};
