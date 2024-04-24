#include "capture_impl.hpp"
#include "smv/log.hpp"
#include "smv/record.hpp"

#include <thread>
#include <variant>

namespace smv {
  using details::ScreenshotSource, details::AudioCaptureSource,
    details::VideoCaptureSource;
  using smv::log::logger;

  void capture(const VideoCaptureConfig      &config,
               TCaptureCb<VideoCaptureSource> callback)
  {
    if (std::holds_alternative<Window *>(config.area) &&
        std::get<Window *>(config.area) == nullptr) {
      logger->warn("No region or window specified for video capture");
      return;
    }
    std::thread([config, callback = std::move(callback)]() {
      std::shared_ptr<VideoCaptureSource> source =
        details::createVideoCaptureSource(config);
      if (source) {
        callback(*source);
      }
    }).detach();
  }

  void capture(const AudioCaptureConfig      &config,
               TCaptureCb<AudioCaptureSource> callback)
  {
    if (config.sourceId == 0) {
      logger->warn("No audio source specified for audio capture");
      return;
    }
    std::thread([config, callback = std::move(callback)]() {
      auto source = details::createAudioCaptureSource(config);
      if (source) {
        callback(*source);
      }
    }).detach();
  }

  void capture(const ScreenshotConfig      &config,
               TCaptureCb<ScreenshotSource> callback)
  {
    if (std::holds_alternative<Window *>(config.area) &&
        std::get<Window *>(config.area) == nullptr) {
      logger->warn("No region or window specified for screenshot");
      return;
    }
    std::thread([config = config, callback = std::move(callback)]() {
      auto source = details::createScreenshotCaptureSource(config);
      if (source) {
        callback(*source);
      }
    }).detach();
  }

  void capture(AudioCaptureConfig config,
               AudioCaptureFormat format,
               CaptureCb          callback)
  {
    capture(config,
            [callback = std::move(callback)](const AudioCaptureSource &) {
      // TODO: capture the audio and call the callback
      logger->warn("Audio capture not yet implemented");
    });
  }

  void capture(ScreenshotConfig config,
               ScreenshotFormat format,
               CaptureCb        callback)
  {
    capture(config,
            [callback = std::move(callback), &format, &config](
              const ScreenshotSource &source) {
      std::optional<ScreenshotSource> formattedSource;

      switch (format) {
        case ScreenshotFormat::PNG: {
          auto pngSource = ScreenshotSource::toPNG(source);
          if (pngSource) {
            formattedSource = pngSource;
          } else {
            logger->error("Failed to convert screenshot to PNG");
          }
          break;
        }
        case ScreenshotFormat::JPG: {
          auto jpgSource = ScreenshotSource::toJPG(source, config.jpegQuality);
          if (jpgSource) {
            formattedSource = jpgSource;
          } else {
            logger->error("Failed to convert screenshot to JPG");
          }
          break;
        }
        case ScreenshotFormat::PPM: {
          auto ppmSource = ScreenshotSource::toPPM(source);
          if (ppmSource) {
            formattedSource = ppmSource;
          } else {
            logger->error("Failed to convert screenshot to PPM");
          }
          break;
        }
        case ScreenshotFormat::QOI: {
          auto qoiSource = ScreenshotSource::toQoi(source);
          if (qoiSource) {
            formattedSource = qoiSource;
          } else {
            logger->error("Failed to convert screenshot to QOI");
          }
          break;
        }
      }
      if (formattedSource) {
        callback(*formattedSource);
      }
    });
  }

  void capture(VideoCaptureConfig config,
               VideoCaptureFormat format,
               CaptureCb          callback)
  {
    capture(config,
            [callback = std::move(callback)](const VideoCaptureSource &) {
      // TODO: capture the video and call the callback
      logger->warn("Video capture not yet implemented");
    });
  }

  void captureStream(VideoStreamConfig config,
                     VideoStreamFormat format,
                     CaptureCb         callback)
  {
    capture(dynamic_cast<VideoCaptureConfig &>(config),
            [callback = std::move(callback)](const VideoCaptureSource &) {
      // TODO: capture the video and call the callback
      logger->warn("Video streaming not yet implemented");
    });
  }

  void captureStream(AudioStreamConfig config,
                     AudioStreamFormat format,
                     CaptureCb         callback)
  {
    capture(dynamic_cast<AudioCaptureConfig &>(config),
            [callback = std::move(callback)](const AudioCaptureSource &) {
      // TODO: capture the audio and call the callback
      logger->warn("Audio streaming not yet implemented");
    });
  }
} // namespace smv
