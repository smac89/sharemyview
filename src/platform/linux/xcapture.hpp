#pragma once

#include "smv/capture_screenshot.hpp"
#include "smv/record.hpp"

#include <cstdint>
#include <mutex>

#include <optional>
#include <xcb/xcb_image.h>

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
