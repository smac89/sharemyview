#pragma once

#include "smv/record.hpp"
#include "smv/window.hpp"

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace smv::details {
  class ScreenshotSource
    : public CaptureSource
    , private Size
  {
  public:
    explicit ScreenshotSource();

    ScreenshotSource(std::variant<std::vector<uint8_t>, std::string> &&data,
                     Size dimension);

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
                     uint8_t scanlinePadding);

    auto error() noexcept -> std::optional<std::string> override;
    auto next() noexcept
      -> std::optional<std::basic_string_view<uint8_t>> override;
    virtual auto width() const noexcept -> uint32_t;
    virtual auto height() const noexcept -> uint32_t;
    virtual auto scanLine() const noexcept -> uint32_t;
    virtual auto channels() const noexcept -> uint8_t;
    virtual auto encoding() const noexcept -> std::optional<ScreenshotFormat>;

    static auto toPNG(ScreenshotSource &source)
      -> std::optional<ScreenshotSource>;

    static auto toJPG(ScreenshotSource &source, int quality)
      -> std::optional<ScreenshotSource>;

    static auto toPPM(ScreenshotSource &source)
      -> std::optional<ScreenshotSource>;

    static auto toQoi(ScreenshotSource & /*unused*/)
      -> std::optional<ScreenshotSource>;

    // allow writeFunc to access private members.
    // See capture_screenshot_impl.cpp
    friend void writeFunc(void *context, void *data, int size);

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

  /**
   * @brief Used by stb_image for writing encoded images
   *
   * @param context the context supplied to the image writing function
   * @param data the data to write
   * @param size the number of bytes to write
   */
  void writeFunc(void *context, void *data, int size);

  auto createScreenshotCaptureSource(const ScreenshotConfig &)
    -> std::shared_ptr<ScreenshotSource>;
  auto createAudioCaptureSource(const AudioCaptureConfig &)
    -> std::shared_ptr<AudioCaptureSource>;
  auto createVideoCaptureSource(const VideoCaptureConfig &)
    -> std::shared_ptr<VideoCaptureSource>;
} // namespace smv::details
