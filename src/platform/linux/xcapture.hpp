#pragma once

#include "smv/capture_screenshot.hpp"
#include "smv/record.hpp"

#include <cstdint>
#include <mutex>

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
 * I think we can use libsoundio/rtaudio for that
 */
namespace smv::details {
  class XRecord
  {
    explicit XRecord();

  public:
    auto screenshot(const decltype(ScreenshotConfig::area) &area)
      -> ScreenshotSource;

    ~XRecord();

    static auto instance() -> XRecord &;

  private:
    std::mutex                            captureLock;
    std::optional<xcb_shm_segment_info_t> shmInfo = std::nullopt;
    uint32_t                              shmSize = 0;
  };

  /**
   * @brief Initialize capture
   *
   * @return true
   * @return false
   */
  auto initCapture() -> bool;

  /**
   * @brief stop capturing streams and cleanup
   */
  void deinitCapture();
} // namespace smv::details
