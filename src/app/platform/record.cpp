#include "smv/platform/capture.hpp"

#include <thread>

namespace smv {
  void capture(CaptureConfig config, CaptureCb callback)
  {
    std::thread([config = std::move(config), callback = std::move(callback)]() {
      std::shared_ptr<CaptureSource> source;
      if (config.screen) {
        if (config.screen->screenMode == ScreenCaptureMode::Single) {
          source = details::createScreenshotCaptureSource(*config.screen);
        } else if (config.screen->screenMode == ScreenCaptureMode::Continuous) {
          source = details::createVideoCaptureSource(config.screen.value(),
                                                     config.audio.value());
        }
      } else if (config.audio) {
        source = details::createAudioCaptureSource(config.audio.value());
      }
      if (source) {
        callback(source);
      }
    }).detach();
  }

  void capture(AudioCaptureConfig config, CaptureCb callback)
  {
    return capture(CaptureConfig { .audio = config }, std::move(callback));
  }

  void capture(ScreenCaptureConfig config, CaptureCb callback)
  {
    return capture(CaptureConfig { .screen = std::move(config) },
                   std::move(callback));
  }
} // namespace smv
