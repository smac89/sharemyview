#pragma once

#include "window.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

constexpr auto DEFAULT_FPS = 40;

namespace smv {
  struct CaptureSource
  {
    virtual auto next() noexcept
      -> std::optional<std::basic_string_view<std::byte>>       = 0;
    virtual auto error() noexcept -> std::optional<std::string> = 0;
    virtual ~CaptureSource()                                    = default;
  };

  using CaptureCb = std::function<void(std::shared_ptr<CaptureSource> source)>;

  enum class ScreenCaptureMode : uint8_t
  {
    Single     = 0x1,
    Continuous = 0x2,
  };

  /**
   * @brief Configure how the screen will be captured
   */
  struct ScreenCaptureConfig
  {
    /**
     * @brief The mode of the screen capture
     */
    ScreenCaptureMode screenMode = ScreenCaptureMode::Single;

    /**
     * @brief The window/region to be captured
     */
    std::variant<Region, Window *> area = nullptr;

    /**
     * @brief A hint for the number of frames per second
     * @details The backend takes this as a hint, but it could be less
     * or more depending on available system resources
     */
    uint8_t fpsHint = DEFAULT_FPS;
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

  struct CaptureConfig
  {
    std::optional<ScreenCaptureConfig> screen;
    std::optional<AudioCaptureConfig>  audio;
  };

  void capture(CaptureConfig config, CaptureCb callback);
  void capture(AudioCaptureConfig config, CaptureCb callback);
  void capture(ScreenCaptureConfig config, CaptureCb callback);
} // namespace smv
