#include "capture_audio.hpp"
#include "capture_impl.hpp"
#include "smv/log.hpp"

#include <thread>
#include <utility>

namespace smv::details {
  using smv::log::logger;

  void capture(const AudioCaptureConfig      &config,
               TCaptureCb<AudioCaptureSource> callback)
  {
    if (config.sourceId == 0) {
      logger->error("No audio source specified for audio capture");
      return;
    }
    std::thread([config, callback = std::move(callback)]() {
      auto source = details::createAudioCaptureSource(config);
      if (source) {
        callback(*source);
      }
    }).detach();
  }
} // namespace smv::details

namespace smv {
  using smv::details::AudioCaptureSource;
  using smv::details::capture;
  using smv::log::logger;

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
