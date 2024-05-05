#include "capture_video.hpp"
#include "capture_impl.hpp"
#include "smv/log.hpp"

#include <memory>
#include <thread>
#include <utility>

namespace smv::details {
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
} // namespace smv::details

namespace smv {
  using smv::details::capture;
  using smv::details::VideoCaptureSource;
  using smv::log::logger;

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
} // namespace smv
