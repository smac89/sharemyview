#pragma once

#include "capture_audio.hpp"
#include "capture_screenshot.hpp"
#include "capture_video.hpp"
#include "smv/record.hpp"

#include <memory>

namespace smv::details {
  auto createScreenshotCaptureSource(const ScreenshotConfig &)
    -> std::shared_ptr<ScreenshotSource>;
  auto createAudioCaptureSource(const AudioCaptureConfig &)
    -> std::shared_ptr<AudioCaptureSource>;
  auto createVideoCaptureSource(const VideoCaptureConfig &)
    -> std::shared_ptr<VideoCaptureSource>;
} // namespace smv::details
