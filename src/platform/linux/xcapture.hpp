#pragma once

#include "smv/record.hpp"

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <optional>
#include <xcb/xcb_image.h>

/**
 * @brief Record screen and audio
 * @details How do we actually record on demand?
 * When a user chooses to record a portion of the screen
 * we create a shared memory buffer to store images captured
 * from the region send back to the user as fast as possible
 *
 * If instead they choose to record window, we need to capture
 * the window's pixmap...
 *
 * TODO:For now, let's just capture a region
 *
 * As far as capturing audio, we will postpone that for now
 * I think we can use libsoundio for that
 */
namespace smv::details {
  struct ScreenshotSource: CaptureSource
  {
    ScreenshotSource(std::variant<std::vector<std::byte>, std::string> data)
    {
      if (std::holds_alternative<std::vector<std::byte>>(data)) {
        captureBytes = std::get<std::vector<std::byte>>(std::move(data));
      } else {
        errMsg = std::get<std::string>(std::move(data));
      }
    }
    auto next() noexcept
      -> std::optional<std::basic_string_view<std::byte>> override
    {
      return std::basic_string_view(captureBytes.data(), captureBytes.size());
    }
    auto error() noexcept -> std::optional<std::string> override
    {
      return errMsg.empty() ? std::nullopt : std::optional(std::string(errMsg));
    }

  private:
    std::string            errMsg;
    std::vector<std::byte> captureBytes = {};
  };

  class XRecord
  {
    explicit XRecord();

  public:
    auto screenshot(decltype(ScreenCaptureConfig::area) area)
      -> ScreenshotSource;

    ~XRecord();

    static auto instance() -> XRecord &;

  private:
    std::mutex                            captureLock;
    std::optional<xcb_shm_segment_info_t> shmInfo = std::nullopt;
    uint32_t                              shmSize = 0;
  };

  /**
   * @brief Initialize recording
   *
   * @return true
   * @return false
   */
  auto initRecord() -> bool;

  /**
   * @brief stop recording streams and cleanup
   */
  void deinitRecord();
} // namespace smv::details
