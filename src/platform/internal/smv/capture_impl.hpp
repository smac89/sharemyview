#pragma once

#include "smv/record.hpp"
#include "smv/window.hpp"

#include <cstdint>
#include <memory>

namespace smv::details {
  class ScreenshotSource
    : public CaptureSource
    , private Size
  {
  public:
    ScreenshotSource() = default;
    ScreenshotSource(std::variant<std::vector<uint8_t>, std::string> data,
                     Size                                            size,
                     uint8_t channelCount = 4)
      : Size(size)
      , channelCount(channelCount)
    {
      if (std::holds_alternative<std::vector<uint8_t>>(data)) {
        captureBytes = std::get<std::vector<uint8_t>>(std::move(data));
      } else {
        errMsg = std::get<std::string>(std::move(data));
      }
    }
    auto next() noexcept
      -> std::optional<std::basic_string_view<std::byte>> override
    {
      return std::basic_string_view(
        reinterpret_cast<std::byte *>(captureBytes.data()),
        captureBytes.size());
    }

    auto width() const noexcept { return w; }
    auto height() const noexcept { return h; }
    auto channels() const noexcept { return channelCount; }
    auto encoding() const noexcept { return format; }

    auto error() noexcept -> std::optional<std::string> override
    {
      return errMsg;
    }

    static auto toPNG(const ScreenshotSource &source)
      -> std::optional<ScreenshotSource>;

    static auto toJPG(const ScreenshotSource &source, int quality)
      -> std::optional<ScreenshotSource>;

    static auto toPPM(const ScreenshotSource &source)
      -> std::optional<ScreenshotSource>;

    static auto toQoi(const ScreenshotSource & /*unused*/)
      -> std::optional<ScreenshotSource>;

  private:
    uint8_t                         channelCount = 0; // 4 for RGBA, 3 for RGB
    std::optional<std::string>      errMsg;
    std::optional<ScreenshotFormat> format = std::nullopt;
    std::vector<uint8_t>            captureBytes {};
  };

  struct AudioCaptureSource: CaptureSource
  {};
  struct VideoCaptureSource: CaptureSource
  {};

  auto createScreenshotCaptureSource(const ScreenshotConfig &)
    -> std::shared_ptr<ScreenshotSource>;
  auto createAudioCaptureSource(const AudioCaptureConfig &)
    -> std::shared_ptr<AudioCaptureSource>;
  auto createVideoCaptureSource(const VideoCaptureConfig &)
    -> std::shared_ptr<VideoCaptureSource>;
} // namespace smv::details
