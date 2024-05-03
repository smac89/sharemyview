#pragma once

#include "window.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

constexpr auto DEFAULT_FPS          = 40;
constexpr auto DEFAULT_JPEG_QUALITY = 95U;

namespace smv {
  struct CaptureSource
  {
    /**
     * @brief Reads a new range of bytes from the underlying source
     * @details The returned range is valid until the next call to next
     * therefore, the caller should copy the data if it needs to keep it
     *
     * @return std::optional<std::basic_string_view<uint8_t>>
     */
    virtual auto inline next() noexcept
      -> std::optional<std::basic_string_view<uint8_t>> = 0;

    /**
     * @brief Returns any error that may have occurred
     *
     * @return std::optional<std::string>
     */
    virtual auto error() noexcept -> std::optional<std::string> = 0;
    virtual ~CaptureSource()                                    = default;
  };

  template<typename S,
           std::enable_if<std::is_base_of_v<CaptureSource, S>> * = nullptr>
  using TCaptureCb = std::function<void(S &source)>;
  using CaptureCb  = TCaptureCb<CaptureSource>;

  // NOTE: Leave these enum values to be stored as ints, because when they are
  // used on the QT side, they are stored as int
  enum class ScreenshotFormat
  {
    PNG  = 0x1,
    JPEG = 0x2,
    PPM  = 0x4,
    QOI  = 0x8,
  };

  enum class AudioCaptureFormat
  {
    OPUS = 0x1,
    AAC  = 0x2,
    MP3  = 0x4,
  };

  enum class AudioStreamFormat
  {
    AAC = 0x1,
  };

  enum class VideoCaptureFormat
  {
    MP4 = 0x1,
    AVI = 0x2,
    GIF = 0x4,
  };

  enum class VideoStreamFormat
  {
    H264 = 0x1,
    H265 = 0x2,
  };

  struct ScreenshotConfig
  {
    /**
     * @brief The window/region to be captured
     */
    std::variant<Window *, Region> area = nullptr;

    uint8_t jpegQuality = DEFAULT_JPEG_QUALITY;

    auto inline isValid() const -> bool
    {
      if (std::holds_alternative<Window *>(area)) {
        const auto *const window = std::get<Window *>(area);
        return window != nullptr && window->isValid();
      }
      return std::get<Region>(area).isValid();
    }
  };

  /**
   * @brief Configure how the audio will be captured
   */
  struct AudioCaptureConfig
  {
    using AudioSourceId = uint32_t;

    /**
     * @brief The source id of the audio
     */
    AudioSourceId sourceId = 0;

    /**
     * @brief The volume of the audio
     * @details The volume is a float between 0.0 and 1.0
     */
    float volume = 1.0F;
  };

  struct VideoCaptureConfig: public ScreenshotConfig
  {
    /**
     * @brief The number of frames per second
     * @details The backend takes this as a hint, but it could be less
     * or more depending on available system resources
     */
    uint8_t fpsHint = DEFAULT_FPS;

    /**
     * @brief The audio configuration
     */
    AudioCaptureConfig audioConfig;
  };

  struct AudioStreamConfig: public AudioCaptureConfig
  {
    std::string rtspUrl;
  };

  struct VideoStreamConfig: public VideoCaptureConfig
  {
    std::string rtspUrl;
  };

  /**
   * @brief Asynchronously start capturing for a video stream
   *
   * @param config The configuration for the capture
   * @param format The format of the capture
   * @param callback The callback to call as capture continues
   */
  void capture(VideoCaptureConfig config,
               VideoCaptureFormat format,
               CaptureCb          callback);

  void capture(AudioCaptureConfig config,
               AudioCaptureFormat format,
               CaptureCb          callback);

  /**
   * @brief Asynchronously start capturing for a screenshot
   *
   * @param config The configuration for the screenshot
   * @param format The format of the screenshot
   * @param callback The callback to call when the screenshot is ready
   */
  void capture(ScreenshotConfig config,
               ScreenshotFormat format,
               CaptureCb        callback);

  void captureStream(VideoStreamConfig config,
                     VideoStreamFormat format,
                     CaptureCb         callback);

  void captureStream(AudioStreamConfig config,
                     AudioStreamFormat format,
                     CaptureCb         callback);
} // namespace smv
