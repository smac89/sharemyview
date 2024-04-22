#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace smv::log {
  extern const std::shared_ptr<spdlog::logger> logger;
  constexpr auto LOGGER_NAME_WINCLIENT = "smv::winclient";
} // namespace smv::log
