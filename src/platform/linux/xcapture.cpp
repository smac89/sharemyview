#include "xcapture.hpp"
#include "smv/capture_impl.hpp"
#include "smv/log.hpp"
#include "smv/record.hpp"
#include "xtools.hpp"
#include "xutils.hpp"
#include "xwindow.hpp"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <mutex>
#include <sys/mman.h>
#include <variant>

#include <assert.hpp>
#include <spdlog/fmt/fmt.h>
#include <xcb/shm.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xproto.h>

constexpr auto SCREENSHOT_ERROR = "Unable to take screenshot";
constexpr auto SHM_CREATE_ERROR = "Failed to create shared memory segment";
constexpr auto CAPTURE_MODULE_UNINITIALIZED =
  "Capture module has not been initialized";
constexpr auto MAX_BYTES_PER_PIXEL = 4U;

namespace smv::details {
  using smv::utils::res, smv::log::logger;
  namespace {
    std::atomic_bool  captureReady = false;
    xcb_image_order_t imageOrder   = XCB_IMAGE_ORDER_LSB_FIRST;

    auto pixelsToVector(const uint8_t *bytes, size_t size)
      -> std::vector<uint8_t>
    {
      ASSERT(
        size % MAX_BYTES_PER_PIXEL == 0, "size must be a multiple of 4", size);
      std::vector<uint8_t> captureBytes {};
      captureBytes.reserve(size);

      for (size_t i = 0; i + MAX_BYTES_PER_PIXEL < size;
           i += MAX_BYTES_PER_PIXEL) {
        // reverse the bytes so that they are in the order blue, green, red
        captureBytes.insert(captureBytes.end(),
                            std::reverse_iterator(&bytes[i + 3]),
                            std::reverse_iterator(&bytes[i]));
      }

      // highly unlikely branch
      if (imageOrder == XCB_IMAGE_ORDER_MSB_FIRST) {
        for (size_t i = 0; i + 3 < captureBytes.size(); i += 3) {
          // swap red and blue so that red is always first
          std::reverse(&captureBytes[i], &captureBytes[i + 3]);
        }
      }
      return captureBytes;
    }

    auto capturePixels(xcb_drawable_t                drawable,
                       const Region *const           region,
                       const xcb_shm_segment_info_t &shmInfo)
      -> std::variant<std::vector<uint8_t>, std::string>
    {
      xcb_generic_error_t                       *err = nullptr;
      std::shared_ptr<xcb_shm_get_image_reply_t> image(xcb_shm_get_image_reply(
        res::connection.get(),
        xcb_shm_get_image(res::connection.get(),
                          drawable,
                          static_cast<int16_t>(region->x()),
                          static_cast<int16_t>(region->y()),
                          region->width(),
                          region->height(),
                          ~0,
                          XCB_IMAGE_FORMAT_Z_PIXMAP,
                          shmInfo.shmseg,
                          0),
        &err));
      std::shared_ptr<xcb_generic_error_t>       _ { err };

      if (err != nullptr) {
        return fmt::format(
          "{}: {}", SCREENSHOT_ERROR, getErrorCodeName(err->error_code));
      }
      return pixelsToVector(shmInfo.shmaddr, image->size);
    }

    auto capturePixels(xcb_drawable_t drawable, const Region *const region)
      -> std::variant<std::vector<uint8_t>, std::string>
    {
      xcb_generic_error_t                   *err = nullptr;
      std::shared_ptr<xcb_get_image_reply_t> image(xcb_get_image_reply(
        res::connection.get(),
        xcb_get_image_unchecked(res::connection.get(),
                                XCB_IMAGE_FORMAT_Z_PIXMAP,
                                drawable,
                                static_cast<int16_t>(region->x()),
                                static_cast<int16_t>(region->y()),
                                region->width(),
                                region->height(),
                                ~0),
        &err));
      std::shared_ptr<xcb_generic_error_t>   _ { err };
      if (err != nullptr) {
        return fmt::format(
          "{}: {}", SCREENSHOT_ERROR, getErrorCodeName(err->error_code));
      }
      auto *data = xcb_get_image_data(image.get());
      auto  size = xcb_get_image_data_length(image.get());
      return pixelsToVector(data, size);
    }
  } // namespace

  XRecord::XRecord()
  {
    const auto *setup = xcb_get_setup(res::connection.get());
    auto        roots = xcb_setup_roots_iterator(setup);

    xcb_shm_seg_t shmseg = xcb_generate_id(res::connection.get());
    // TODO: Initialize for multiple screens or use a value big enough for all
    // screens
    auto shmSize = roots.data->width_in_pixels * roots.data->height_in_pixels *
                   MAX_BYTES_PER_PIXEL;

    auto shm_reply = std::unique_ptr<xcb_shm_create_segment_reply_t>(
      xcb_shm_create_segment_reply(
        res::connection.get(),
        xcb_shm_create_segment_unchecked(
          res::connection.get(), shmseg, shmSize, 0U),
        nullptr));

    if (!shm_reply) {
      logger->error("[XRecord]: {}. Size={}", SHM_CREATE_ERROR, shmSize);
      return;
    }

    if (shm_reply->nfd != 1) {
      logger->error("[XRecord]: {}. Invalid number of fds: {}",
                    SHM_CREATE_ERROR,
                    shm_reply->nfd);
      return;
    }

    auto *fds =
      xcb_shm_create_segment_reply_fds(res::connection.get(), shm_reply.get());

    auto *buffer =
      mmap(nullptr, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, *fds, 0);
    close(*fds);
    if (buffer == MAP_FAILED) {
      logger->error("[XRecord]: {}. MMAP failed: {}",
                    SHM_CREATE_ERROR,
                    std::strerror(errno));
      return;
    }
    xcb_shm_segment_info_t shmInfo {};
    shmInfo.shmseg  = shmseg;
    shmInfo.shmaddr = static_cast<uint8_t *>(buffer);
    this->shmInfo   = shmInfo;
    this->shmSize   = shmSize;
  }

  XRecord::~XRecord()
  {
    if (shmInfo) {
      munmap(shmInfo->shmaddr, shmSize);
      shmInfo = std::nullopt;
      shmSize = 0;
    }
  }

  auto XRecord::screenshot(const decltype(ScreenshotConfig::area) &area)
    -> ScreenshotSource
  {
    xcb_drawable_t root   = 0;
    const Region  *region = nullptr;

    if (std::holds_alternative<Window *>(area)) {
      auto *window = dynamic_cast<XWindow *>(std::get<Window *>(area));
      root         = window->id();
      region       = window;
    } else {
      const auto *setup      = xcb_get_setup(res::connection.get());
      auto        roots_iter = xcb_setup_roots_iterator(setup);
      root                   = roots_iter.data->root;
      region                 = &std::get<Region>(area);
    }

    if (shmInfo) {
      std::lock_guard _(captureLock);
      return { capturePixels(root, region, *shmInfo), region->size() };
    }
    return { capturePixels(root, region), region->size() };
  }

  auto XRecord::instance() -> XRecord &
  {
    static XRecord instance;
    return instance;
  }

  auto initCapture() -> bool
  {
    // TODO: when capturing a region which may be in a different screen
    // we need to use the correct root window
    // @see xscreen from xutils.hpp
    const auto *setup = xcb_get_setup(res::connection.get());
    imageOrder        = static_cast<xcb_image_order_t>(setup->image_byte_order);

    captureReady = true;
    return true;
  }

  void deinitCapture()
  {
    captureReady = false;
  }

  auto createScreenshotCaptureSource(const ScreenshotConfig &config)
    -> std::shared_ptr<ScreenshotSource>
  {
    if (!captureReady) {
      logger->warn(CAPTURE_MODULE_UNINITIALIZED);
      return nullptr;
    }

    auto &instance = XRecord::instance();
    return std::make_shared<ScreenshotSource>(instance.screenshot(config.area));
  }

  auto createAudioCaptureSource(const AudioCaptureConfig & /*unused*/)
    -> std::shared_ptr<AudioCaptureSource>
  {
    if (!captureReady) {
      logger->warn(CAPTURE_MODULE_UNINITIALIZED);
      return nullptr;
    }
    return nullptr;
  }

  auto createVideoCaptureSource(const VideoCaptureConfig & /*unused*/)
    -> std::shared_ptr<VideoCaptureSource>
  {
    if (!captureReady) {
      logger->warn(CAPTURE_MODULE_UNINITIALIZED);
      return nullptr;
    }
    return nullptr;
  }
} // namespace smv::details
