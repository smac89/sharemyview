#include "capture_impl.hpp"
#include "smv/common/c_iter.hpp"
#include "smv/log.hpp"
#include "smv/record.hpp"

#include <cstdint>
#include <iterator>
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

  ScreenshotSource::ScreenshotSource()
    : ScreenshotSource({}, { 0, 0 })
  {
  }

  ScreenshotSource::ScreenshotSource(
    std::variant<std::vector<uint8_t>, std::string> &&data,
    Size                                              dimension)
    : ScreenshotSource(std::move(data), dimension, 3, 1, 0)
  {
  }

  ScreenshotSource::ScreenshotSource(
    std::variant<std::vector<uint8_t>, std::string> &&data,
    Size                                              dimension,
    uint8_t                                           channelCount,
    uint8_t                                           bytesPerPixel,
    uint8_t                                           scanlinePadding)
    : Size(dimension)
    , channelCount(channelCount)
    , bytesPerPixel(bytesPerPixel)
    , scanlinePaddingBytes(scanlinePadding)
  {
    if (std::holds_alternative<std::vector<uint8_t>>(data)) {
      captureBytes = std::get<std::vector<uint8_t>>(std::move(data));
    } else {
      errMsg = std::get<std::string>(std::move(data));
      logger->debug("Failed to capture screenshot: {}", errMsg.value());
    }
  }

  auto ScreenshotSource::next() noexcept
    -> std::optional<std::basic_string_view<uint8_t>>
  {
    if (readPos >= captureBytes.size()) {
      return std::nullopt;
    }
    auto bytes =
      std::basic_string_view(captureBytes.data(), captureBytes.size());
    readPos = bytes.size();
    return bytes;
  }

  auto ScreenshotSource::width() const noexcept -> uint32_t
  {
    return w;
  }
  auto ScreenshotSource::height() const noexcept -> uint32_t
  {
    return h;
  }
  auto ScreenshotSource::scanLine() const noexcept -> uint32_t
  {
    return bytesPerPixel * channelCount * w + scanlinePaddingBytes;
  }
  auto ScreenshotSource::channels() const noexcept -> uint8_t
  {
    return channelCount;
  }
  auto ScreenshotSource::encoding() const noexcept
    -> std::optional<ScreenshotFormat>
  {
    return format;
  }

  auto ScreenshotSource::error() noexcept -> std::optional<std::string>
  {
    return errMsg;
  }

  auto ScreenshotSource::toPNG(ScreenshotSource &source)
    -> std::optional<ScreenshotSource>
  {
    ScreenshotSource pngSource;
    if (stbi_write_png_to_func(&writeFunc,
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

  auto ScreenshotSource::toJPG(ScreenshotSource &source, int quality)
    -> std::optional<ScreenshotSource>
  {
    ScreenshotSource jpgSource {};
    if (stbi_write_jpg_to_func(&writeFunc,
                               &jpgSource,
                               static_cast<int>(source.width()),
                               static_cast<int>(source.height()),
                               source.channels(),
                               source.captureBytes.data(),
                               quality)) {
      jpgSource.format       = ScreenshotFormat::JPEG;
      jpgSource.w            = source.width();
      jpgSource.h            = source.height();
      jpgSource.channelCount = source.channels();
      return jpgSource;
    }
    return std::nullopt;
  }

  auto ScreenshotSource::toPPM(ScreenshotSource &source)
    -> std::optional<ScreenshotSource>
  {
    if (source.channelCount < 3) {
      return std::nullopt;
    }
    /* TODO: Replace with Boost.Interprocess, specifically vectorstream */
    for (size_t i = 0; i + source.channelCount < source.captureBytes.size();
         i += source.channelCount) {
      auto blue = source.captureBytes[i];
      auto red  = source.captureBytes[i + 2];
      std::swap(red, blue);
    }
    ScreenshotSource ppmSource;
    ppmSource.format       = ScreenshotFormat::PPM;
    ppmSource.w            = source.width();
    ppmSource.h            = source.height();
    ppmSource.channelCount = source.channelCount;

    std::stringstream oss;
    oss << "P6\n"
        << source.width() << "\n"
        << source.height() << "\n255\n"
        << source.captureBytes.data();
    std::vector<uint8_t> captureBytes(std::move(source.captureBytes));
    captureBytes.resize(oss.flush().tellp());
    std::copy(std::istreambuf_iterator(oss),
              std::istreambuf_iterator<char> {},
              captureBytes.begin());

    ppmSource.captureBytes = std::move(captureBytes);
    return ppmSource;
  }

  auto ScreenshotSource::toQoi(ScreenshotSource & /*unused*/)
    -> std::optional<ScreenshotSource>
  {
    // TODO
    logger->warn("QOI conversion not yet implemented");
    return std::nullopt;
  }

  void writeFunc(void *context, void *data, int size)
  {
    // NOTE: writeFunc can be called multiple times, which is why we
    // have these checks to determine how much more memory to reserve
    auto *dest = static_cast<ScreenshotSource *>(context);
    if (dest->captureBytes.empty()) {
      dest->captureBytes.reserve(size);
    } else if (dest->captureBytes.size() + size >=
               dest->captureBytes.capacity()) {
      if (dest->captureBytes.size() >= static_cast<size_t>(size)) {
        dest->captureBytes.reserve(dest->captureBytes.size() << 1);
      } else {
        dest->captureBytes.reserve(dest->captureBytes.size() + (size << 1));
      }
    }
    auto iter =
      utils::CPtrIterator<uint8_t>(static_cast<uint8_t *>(data), size);
    dest->captureBytes.insert(
      dest->captureBytes.end(), iter.begin(), iter.end());
  }
} // namespace smv::details
