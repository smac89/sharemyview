#pragma once

#include "window.hpp"

#include <cstddef>
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
    virtual auto inline next() noexcept
      -> std::optional<std::basic_string_view<std::byte>>       = 0;
    virtual auto error() noexcept -> std::optional<std::string> = 0;
    virtual ~CaptureSource()                                    = default;
  };

  template<typename S,
           std::enable_if<std::is_base_of_v<CaptureSource, S>> * = nullptr>
  using TCaptureCb = std::function<void(const S &source)>;
  using CaptureCb  = TCaptureCb<CaptureSource>;

  enum class ScreenshotFormat : uint8_t
  {
    PPM = 0x1,
    PNG = 0x2,
    JPG = 0x4,
    QOI = 0x8,
  };

  enum class AudioCaptureFormat : uint8_t
  {
    OPUS = 0x1,
    AAC  = 0x2,
    MP3  = 0x4,
  };

  enum class AudioStreamFormat : uint8_t
  {
    AAC = 0x1,
  };

  enum class VideoCaptureFormat : uint8_t
  {
    MP4 = 0x1,
    AVI = 0x2,
  };

  enum class VideoStreamFormat : uint8_t
  {
    H264 = 0x1,
    H265 = 0x2,
  };

  struct ScreenshotConfig
  {
    /**
     * @brief The window/region to be captured
     */
    std::variant<Region, Window *> area = nullptr;

    uint8_t jpegQuality = DEFAULT_JPEG_QUALITY;
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

  void capture(VideoCaptureConfig config,
               VideoCaptureFormat format,
               CaptureCb          callback);

  void capture(AudioCaptureConfig config,
               AudioCaptureFormat format,
               CaptureCb          callback);

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
