#include "capture_impl.hpp"
#include "smv/common/c_iter.hpp"
#include "smv/log.hpp"
#include "smv/record.hpp"

#include <sstream>

extern "C"
{
  // not sure if this is right, however it is a C library, so
  // this is just make sure C++ does not mangle with it
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#endif
}

namespace smv::details {
  using log::logger;

  auto ScreenshotSource::toPNG(const ScreenshotSource &source)
    -> std::optional<ScreenshotSource>
  {
    ScreenshotSource pngSource;
    auto             writeFunc = [](void *context, void *data, int size) {
      auto *dest = static_cast<ScreenshotSource *>(context);
      dest->captureBytes.reserve(size);
      auto iter =
        utils::CPtrIterator<uint8_t>(static_cast<uint8_t *>(data), size);
      dest->captureBytes.insert(
        dest->captureBytes.begin(), iter.begin(), iter.end());
    };

    if (stbi_write_png_to_func(writeFunc,
                               &pngSource,
                               static_cast<int>(source.width()),
                               static_cast<int>(source.height()),
                               source.channels(),
                               source.captureBytes.data(),
                               static_cast<int>(source.scanLine()))) {
      pngSource.format       = ScreenshotFormat::PNG;
      pngSource.w            = source.width();
      pngSource.h            = source.height();
      pngSource.channelCount = source.channels();
      return pngSource;
    }
    return std::nullopt;
  }

  auto ScreenshotSource::toJPG(const ScreenshotSource &source, int quality)
    -> std::optional<ScreenshotSource>
  {
    // TODO: Create an anonymous class that wraps stb_image_write_jpg
    // and return an instance of that, so that the conversion is lazy?

    ScreenshotSource jpgSource;
    auto             writeFunc = [](void *context, void *data, int size) {
      auto *dest = static_cast<ScreenshotSource *>(context);
      dest->captureBytes.reserve(size);
      auto iter =
        utils::CPtrIterator<uint8_t>(static_cast<uint8_t *>(data), size);
      dest->captureBytes.insert(
        dest->captureBytes.begin(), iter.begin(), iter.end());
    };
    if (stbi_write_jpg_to_func(writeFunc,
                               &jpgSource,
                               static_cast<int>(source.width()),
                               static_cast<int>(source.height()),
                               source.channels(),
                               source.captureBytes.data(),
                               quality)) {
      jpgSource.format       = ScreenshotFormat::JPG;
      jpgSource.w            = source.width();
      jpgSource.h            = source.height();
      jpgSource.channelCount = source.channels();
      return jpgSource;
    }
    return std::nullopt;
  }

  auto ScreenshotSource::toPPM(const ScreenshotSource &source)
    -> std::optional<ScreenshotSource>
  {
    if (source.channelCount < 3) {
      return std::nullopt;
    }
    ScreenshotSource ppmSource;
    /* TODO: Replace with Boost.Interprocess, specifically vectorstream */
    std::basic_ostringstream<uint8_t> oss;
    oss << "P6\n" << source.width() << " " << source.height() << "\n255\n";
    auto headerSize = oss.str().size();
    for (size_t i = 0; i + source.channelCount < source.captureBytes.size();
         i += source.channelCount) {
      // TODO: use endianness of the system
      auto blue  = source.captureBytes[i];
      auto green = source.captureBytes[i + 1];
      auto red   = source.captureBytes[i + 2];
      oss << red << green << blue;
    }

    ppmSource.format       = ScreenshotFormat::PPM;
    ppmSource.w            = source.width();
    ppmSource.h            = source.height();
    ppmSource.channelCount = source.channelCount;
    auto &captureBytes     = ppmSource.captureBytes;
    captureBytes.reserve(3UL * source.width() * source.height() + headerSize);
    captureBytes.insert(captureBytes.begin(),
                        std::istreambuf_iterator<uint8_t>(oss.rdbuf()),
                        std::istreambuf_iterator<uint8_t>());

    return ppmSource;
  }

  auto ScreenshotSource::toQoi(const ScreenshotSource & /*unused*/)
    -> std::optional<ScreenshotSource>
  {
    // TODO
    logger->warn("QOI conversion not yet implemented");
    return std::nullopt;
  }
} // namespace smv::details
