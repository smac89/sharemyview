#pragma once

#include "smv/record.hpp"

namespace smv::details {
  struct VideoCaptureSource: public CaptureSource
  {};

  struct Gif89aCaptureSource: public VideoCaptureSource
  {
    /* GIF89a specification: https://www.w3.org/Graphics/GIF/spec-gif89a.txt
    https://www.fileformat.info/format/gif/egff.htm
    */
    constexpr static auto gifVersion = "GIF89a";
  };
} // namespace smv::details
