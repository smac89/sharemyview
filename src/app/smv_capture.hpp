#pragma once

#include "smv/record.hpp"

#include <cstdint>
#include <cstring>
#include <string_view>

#include <QDataStream>
#include <QIODevice>
#include <QRect>
#include <spdlog/spdlog.h>

/**
 * @brief wraps a smv::CaptureSource into a QIODevice
 */
class CaptureSourceIO: public QIODevice
{
public:
  explicit CaptureSourceIO(smv::CaptureSource *source)
    : mCaptureSource(source)
  {
    if (auto image = mCaptureSource->next()) {
      current = *image;
      open(QIODevice::ReadOnly);
    }
  }

  auto isSequential() const -> bool override { return true; }

protected:
  auto readData(char *data, qint64 maxSize) -> qint64 override
  {
    if (current.empty() || maxSize <= 0) {
      return 0;
    }
    auto size = qMin(maxSize, static_cast<qint64>(current.size()));
    std::memcpy(data, current.data(), size);
    current.remove_prefix(size);
    if (current.empty()) {
      if (auto image = mCaptureSource->next()) {
        current = *image;
        if (size < maxSize) {
          return size + readData(&data[size], maxSize - size);
        }
      }
    }
    return size;
  }

  auto writeData(const char * /*data*/, qint64 /*maxSize*/) -> qint64 override
  {
    // Writes are not supported.
    return -1;
  }

private:
  std::basic_string_view<uint8_t> current;
  smv::CaptureSource *const       mCaptureSource;
};
