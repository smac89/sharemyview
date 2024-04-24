#include "xcapture.hpp"
#include "smv/capture_impl.hpp"
#include "smv/common/c_iter.hpp"
#include "smv/log.hpp"
#include "smv/record.hpp"
#include "xtools.hpp"
#include "xutils.hpp"
#include "xwindow.hpp"

#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <sys/mman.h>
#include <variant>

#include <spdlog/fmt/fmt.h>
#include <xcb/shm.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

constexpr auto SCREENSHOT_ERROR = "Unable to take screenshot";
constexpr auto SHM_CREATE_ERROR = "Failed to create shared memory segment";
constexpr auto CAPTURE_MODULE_UNINITIALIZED =
  "Capture module has not been initialized";

namespace smv::details {
  using smv::utils::res, smv::log::logger;
  namespace {
    std::atomic_bool captureReady = false;
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
    utils::CPtrIterator<uint8_t> imageIterator(shmInfo.shmaddr, image->size);
    std::vector<uint8_t>         captureBytes(image->size);
    captureBytes.insert(
      captureBytes.begin(), imageIterator.begin(), imageIterator.end());
    return captureBytes;
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
    auto                        *data = xcb_get_image_data(image.get());
    auto                         size = xcb_get_image_data_length(image.get());
    utils::CPtrIterator<uint8_t> imageIterator(data, size);
    std::vector<uint8_t>         captureBytes(size);
    captureBytes.insert(
      captureBytes.begin(), imageIterator.begin(), imageIterator.end());
    return captureBytes;
  }

  XRecord::XRecord()
  {
    const auto *setup = xcb_get_setup(res::connection.get());
    auto        roots = xcb_setup_roots_iterator(setup);

    xcb_shm_seg_t shmseg = xcb_generate_id(res::connection.get());
    auto          shmSize =
      roots.data->width_in_pixels * roots.data->height_in_pixels * 4;

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

    std::shared_ptr<int> fds(
      xcb_shm_create_segment_reply_fds(res::connection.get(), shm_reply.get()));

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

  auto XRecord::screenshot(decltype(ScreenshotConfig::area) area)
    -> ScreenshotSource
  {
    xcb_drawable_t root   = 0;
    Region        *region = nullptr;

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
      return { capturePixels(root, region, shmInfo.value()), region->size() };
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
    // TODO: something more sophisticated...maybe?
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
    return std::make_shared<ScreenshotSource>(
      XRecord::instance().screenshot(config.area));
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
