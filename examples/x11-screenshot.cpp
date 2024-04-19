#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>

#include <sys/mman.h>
#include <xcb/shm.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xproto.h>

#include <spdlog/spdlog.h>

const auto IMAGE_WIDTH  = 640;
const auto IMAGE_HEIGHT = 480;
const auto SHM_SIZE     = 4 * 1024 * 1024;

// https://stackoverflow.com/questions/56562823/xcb-get-image-reply-fails-for-window-on-another-workspace-desktop

auto main() -> int
{
  /* XCB CONNECTION STUFF */
  std::unique_ptr<xcb_connection_t, decltype(&xcb_disconnect)> c(
    xcb_connect(nullptr, nullptr), &xcb_disconnect);

  if (!c) {
    spdlog::error("failed to connect to X server");
    return EXIT_FAILURE;
  }

  auto *setup      = xcb_get_setup(c.get());
  auto  roots_iter = xcb_setup_roots_iterator(setup);
  if (roots_iter.rem == 0) {
    spdlog::error("no screen found");
    return EXIT_FAILURE;
  }

  auto          root   = roots_iter.data->root;
  xcb_shm_seg_t shmseg = xcb_generate_id(c.get());

  /* SETUP SHARED MEMORY */
  auto shm_reply = std::unique_ptr<xcb_shm_create_segment_reply_t>(
    xcb_shm_create_segment_reply(
      c.get(),
      xcb_shm_create_segment_unchecked(c.get(), shmseg, SHM_SIZE, false),
      nullptr));

  if (!shm_reply) {
    spdlog::error("failed to create shared memory segment");
    return EXIT_FAILURE;
  }

  auto fds = xcb_shm_create_segment_reply_fds(c.get(), shm_reply.get());
  if (shm_reply->nfd != 1) {
    for (int i = 0; i < shm_reply->nfd; i++) {
      close(fds[i]);
    }
    // xcb_shm_attach_fd(xcb_connection_t *c, xcb_shm_seg_t shmseg, int32_t
    // shm_fd, uint8_t read_only); close_range(fds[0], fds[shm_reply->nfd - 1],
    // 0);
    spdlog::error("failed to create shared memory segment");
    return EXIT_FAILURE;
  }

  /* MAP SHARED MEMORY */
  std::shared_ptr<uint8_t> data(
    static_cast<uint8_t *>(
      mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fds[0], 0)),
    [](uint8_t *p) {
    if (p != MAP_FAILED) {
      munmap(p, SHM_SIZE);
    }
  });

  close(fds[0]);
  std::ignore = shm_reply.release();

  if (data.get() == MAP_FAILED) {
    spdlog::error("failed to map shared memory segment");
    xcb_shm_detach(c.get(), shmseg);
    return EXIT_FAILURE;
  }

  std::memset(data.get(), 0, SHM_SIZE);

  // auto pixmap = xcb_generate_id(c.get());
  // xcb_shm_create_pixmap(c.get(),
  //                       pixmap,
  //                       root,
  //                       IMAGE_WIDTH,
  //                       IMAGE_HEIGHT,
  //                       roots_iter.data->root_depth,
  //                       shmseg,
  //                       0);

  /* GET IMAGE OF ROOT WINDOW AND STORE IT IN SHARED MEMORY */
  std::shared_ptr<xcb_shm_get_image_reply_t> image(xcb_shm_get_image_reply(
    c.get(),
    xcb_shm_get_image_unchecked(c.get(),
                                root,
                                0,
                                0,
                                IMAGE_WIDTH,
                                IMAGE_HEIGHT,
                                ~0,
                                XCB_IMAGE_FORMAT_Z_PIXMAP,
                                shmseg,
                                0),
    nullptr));

  if (image) {
    if (auto f = std::ofstream("/tmp/screenshot.ppm", std::ios::binary); f) {
      spdlog::info("Image size: {}", image->size);
      /* WRITE IMAGE TO FILE IN PPM FORMAT */
      f << "P6\n" << IMAGE_WIDTH << " " << IMAGE_HEIGHT << "\n255\n";
      const auto *const imageData = data.get();
      for (uint32_t pixel = 0; pixel + 4 <= image->size; pixel += 4) {
        auto b = imageData[pixel];
        auto g = imageData[pixel + 1];
        auto r = imageData[pixel + 2];
        if (setup->image_byte_order == XCB_IMAGE_ORDER_MSB_FIRST) {
          std::swap(b, r);
        }
        f << r << g << b;
      }
    } else {
      spdlog::error("failed to open file");
    }
  } else {
    spdlog::error("failed to get image");
  }
  xcb_shm_detach(c.get(), shmseg);
  return 0;
}
