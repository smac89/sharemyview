#include "smv_utils.hpp"

auto rectToRegion(const QRect &rect) -> smv::Region
{
  return { static_cast<uint32_t>(rect.width()),
           static_cast<uint32_t>(rect.height()),
           rect.x(),
           rect.y() };
}
