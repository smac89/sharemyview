#pragma once
#include "smv/events.hpp"

#include <QObject>
#include <QWindow>
#include <memory>
#include <shared_mutex>

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

class App: public QObject
{
  // https://doc.qt.io/qt-5/qtqml-cppintegration-exposecppattributes.html
  Q_OBJECT
  Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
  Q_PROPERTY(std::shared_ptr<smv::Window> targetWindow READ targetWindow WRITE
               setTargetWindow NOTIFY targetWindowChanged)
public:
  explicit App(QObject *parent = nullptr);

  enum class Mode
  {
    Region,
    Window
  };
  Q_ENUM(Mode)

  Mode                         mode() const;
  void                         setMode(const Mode);
  std::shared_ptr<smv::Window> targetWindow() const;
  void setTargetWindow(const std::shared_ptr<smv::Window>);
  void operator()(const smv::EventDataMouseEnter &data);

  ~App();

signals:
  void modeChanged(const Mode);
  void targetWindowMoved(const QPoint &);
  void targetWindowResized(const QSize &);
  void targetWindowChanged(const QSize &, const QPoint &);

public slots:
  void updateRecordRegion(const QRect &rect);
  void updateRecordRegion(const QPoint &);
  void updateRecordRegion(const QSize &);
  void updateRecordRegion(const QSize &, const QPoint &);

private slots:
  void takeScreenShot();
  void startRecording();
  void streamRecording();

private:
  QRect                        mRecordRegion;
  Mode                         mMode = Mode::Window;
  smv::Cancel                  mCancel;
  std::shared_mutex            mMutex;
  std::weak_ptr<smv::Window>   mActiveWindow;
  std::shared_ptr<smv::Window> mTargetWindow;
};
