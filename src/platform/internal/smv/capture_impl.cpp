#include "capture_impl.hpp"
#include "smv/common/fmt.hpp" // IWYU pragma: keep
#include "smv/log.hpp"
#include "smv/record.hpp"

#include <optional>
#include <thread>

namespace smv {
  using details::ScreenshotSource, details::AudioCaptureSource,
    details::VideoCaptureSource;
  using smv::log::logger;

  void capture(const VideoCaptureConfig      &config,
               TCaptureCb<VideoCaptureSource> callback)
  {
    if (!config.isValid()) {
      logger->warn("Invalid capture config");
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
    if (!config.isValid()) {
      logger->warn("Invalid capture config");
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
            [callback = std::move(callback),
             format,
             config = std::move(config)](ScreenshotSource &source) {
      std::optional<ScreenshotSource> formattedSource = std::nullopt;
      switch (format) {
        case ScreenshotFormat::PNG: {
          logger->info("Converting screenshot to PNG");
          auto pngSource = ScreenshotSource::toPNG(source);
          if (pngSource) {
            formattedSource = std::move(pngSource);
          } else {
            logger->error("Failed to convert screenshot to PNG");
          }
          break;
        }
        case ScreenshotFormat::JPEG: {
          logger->info("Converting screenshot to JPG");
          auto jpgSource = ScreenshotSource::toJPG(source, config.jpegQuality);
          if (jpgSource) {
            formattedSource = std::move(jpgSource);
          } else {
            logger->error("Failed to convert screenshot to JPG");
          }
          break;
        }
        case ScreenshotFormat::PPM: {
          logger->info("Converting screenshot to PPM");
          auto ppmSource = ScreenshotSource::toPPM(source);
          if (ppmSource) {
            formattedSource = std::move(ppmSource);
          } else {
            logger->error("Failed to convert screenshot to PPM");
          }
          break;
        }
        case ScreenshotFormat::QOI: {
          logger->info("Converting screenshot to QOI");
          auto qoiSource = ScreenshotSource::toQoi(source);
          if (qoiSource) {
            formattedSource = std::move(qoiSource);
          } else {
            logger->error("Failed to convert screenshot to QOI");
          }
          break;
        }
        default:
          logger->error("Unsupported screenshot format: {}", format);
          break;
      }
      if (formattedSource) {
        callback(*formattedSource);
      } else {
        callback(source);
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
