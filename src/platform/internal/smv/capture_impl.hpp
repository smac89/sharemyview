#pragma once

#include "smv/log.hpp"
#include "smv/record.hpp"
#include "smv/window.hpp"

#include <cstdint>
#include <memory>
#include <string_view>

namespace smv::details {
  using smv::log::logger;

  class ScreenshotSource
    : public CaptureSource
    , private Size
  {
  public:
    ScreenshotSource()
      : ScreenshotSource({}, { 0, 0 })
    {
    }

    ScreenshotSource(std::variant<std::vector<uint8_t>, std::string> &&data,
                     Size dimension)
      : ScreenshotSource(std::move(data), dimension, 3, 1, 0)
    {
    }

    /**
     * @brief Construct a new Screenshot Source object
     *
     * @details We make a some assumptions about the format of the data
     * that is being captured. We assume that the data might have a padding
     * between scanlines.
     * Scanline defines the length of a row of pixels. The padding tells how
     * many extra bytes are added to each row
     * Many of these assumptions come from X11
     *
     * @param captureBytes The bytes that are captured
     *
     * @param data The data that is captured
     * @param dimension The dimensions of the image
     * @param channelCount The number of channels in the image (2 = gray, 3 =
     * RGB, 4 = RGBA). Defaults to 3 beause that is what X11 uses
     * @param bytesPerPixel The number of bytes per pixel
     * @param scanlinePadding The padding between scanlines.
     */
    ScreenshotSource(std::variant<std::vector<uint8_t>, std::string> &&data,
                     Size    dimension,
                     uint8_t channelCount,
                     uint8_t bytesPerPixel,
                     uint8_t scanlinePadding)
      : Size(dimension)
      , channelCount(channelCount)
      , bytesPerPixel(bytesPerPixel)
      , scanlinePaddingBytes(scanlinePadding)
    {
      if (std::holds_alternative<std::vector<uint8_t>>(data)) {
        captureBytes = std::get<std::vector<uint8_t>>(std::move(data));
      } else {
        errMsg = std::get<std::string>(std::move(data));
        logger->debug("Failed to capture screenshot: {}", errMsg.value());
      }
    }

    auto next() noexcept
      -> std::optional<std::basic_string_view<uint8_t>> override
    {
      if (readPos >= captureBytes.size()) {
        return std::nullopt;
      }
      auto bytes =
        std::basic_string_view(captureBytes.data(), captureBytes.size());
      readPos = bytes.size();
      return bytes;
    }

    virtual auto width() const noexcept -> uint32_t { return w; }
    virtual auto height() const noexcept -> uint32_t { return h; }
    virtual auto scanLine() const noexcept -> uint32_t
    {
      return bytesPerPixel * channelCount * w + scanlinePaddingBytes;
    }
    virtual auto channels() const noexcept -> uint8_t { return channelCount; }
    virtual auto encoding() const noexcept -> std::optional<ScreenshotFormat>
    {
      return format;
    }

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
    uint8_t                         channelCount;
    uint8_t                         bytesPerPixel;
    uint8_t                         scanlinePaddingBytes;
    std::optional<std::string>      errMsg;
    std::optional<ScreenshotFormat> format  = std::nullopt;
    uint64_t                        readPos = 0;
    std::vector<uint8_t>            captureBytes;
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
