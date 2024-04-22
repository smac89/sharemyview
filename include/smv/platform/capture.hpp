#pragma once

#include "smv/record.hpp"

namespace smv::details {
  auto createScreenshotCaptureSource(const ScreenCaptureConfig &)
    -> std::shared_ptr<CaptureSource>;
  auto createAudioCaptureSource(const AudioCaptureConfig &)
    -> std::shared_ptr<CaptureSource>;
  auto createVideoCaptureSource(const ScreenCaptureConfig &,
                                const AudioCaptureConfig &)
    -> std::shared_ptr<CaptureSource>;
} // namespace smv::details
