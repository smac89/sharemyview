#pragma once
#include "smv/events.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <utility>

#include <assert.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

constexpr auto LOGGER_NAME_AUTOCANCEL = "smv::autocancel";

namespace smv::utils {
  class AutoCancel
  {
    explicit AutoCancel(Cancel &&cancel, uint32_t id)
      : mCancel(std::move(cancel))
      , mId(id)
    {
      logger->debug("Created id: {}", id);
    }

  public:
    AutoCancel(const AutoCancel &other)
      : mCancel(other.mCancel)
      , mId(other.mId)
    {
      std::lock_guard lk(cancelablesMut);
      if (auto ref = cancelables.find(other.mId); ref != cancelables.end()) {
        ref->second++;
        logger->debug("Copy created via constructor id: {}", mId);
      }
    }
    auto operator=(const AutoCancel &other) -> AutoCancel &
    {
      if (mId != other.mId) {
        this->tryCancel();
        {
          std::lock_guard _(cancelablesMut);
          if (auto ref = cancelables.find(other.mId);
              ref != cancelables.end()) {
            ref->second++;
          }
        }
        this->mCancel = other.mCancel;
        this->mId     = other.mId;
        logger->debug("Copy created via assignment id: {}", mId);
      }
      return *this;
    }
    AutoCancel(AutoCancel &&other) = default;
    auto operator=(AutoCancel &&other) noexcept -> AutoCancel &
    {
      if (mId != other.mId) {
        this->tryCancel();
        this->mCancel = std::move(other.mCancel);
        this->mId     = other.mId;
        logger->debug("Move assignment id: {}", mId);
      }
      return *this;
    }
    auto id() const -> uint32_t { return mId; }
    ~AutoCancel() { tryCancel(true); }
    void operator()()
    {
      std::lock_guard _(cancelablesMut);
      if (hasCancel()) {
        doCancel();
      }
    }

  protected:
    void doCancel(bool isDestroy = false)
    {
      logger->debug("destroy id: {}", mId);

      mCancel();
      mCancel = nullptr;
      if (!isDestroy) {
        // prevent double free
        logger->debug("untracking id: {}", mId);
        cancelables.erase(mId);
      }
      logger->debug("cancelled id: {}", mId);
      mId = 0;
    }

    auto tryCancel(bool isDestroy = false) -> bool
    {
      if (!hasCancel()) {
        return false;
      }

      std::lock_guard _(cancelablesMut);
      auto const     &ref = cancelables.find(mId);
      if (ref == cancelables.end()) {
        // this may happen if the function was already called
        return false;
      }

      if (ref->second > 0) {
        ref->second--;
        return false;
      }

      VERIFY(/* NOLINT */
             "No refs",
             cancelables.at(mId) == 0);
      doCancel(isDestroy);
      return true;
    }

    /**
     * @brief check if the cancel function is valid
     * @details if the cancel function is not valid, it means the object was
     * constructed from another object which was moved or already destroyed
     *
     * @return true if the cancel function is valid
     */
    auto hasCancel() const noexcept -> bool { return mCancel != nullptr; }

  private:
    Cancel                                               mCancel;
    uint32_t                                             mId;
    inline static std::atomic_uint32_t                   ids { 1 };
    inline static std::unordered_map<uint32_t, uint32_t> cancelables;
    inline static std::recursive_mutex                   cancelablesMut;
    inline static std::shared_ptr<spdlog::logger>        logger =
      spdlog::stderr_color_mt(LOGGER_NAME_AUTOCANCEL);

  public:
    static auto wrap(Cancel cancel) -> AutoCancel
    {
      // TODO: check if Cancel is just a wrapper for autocancel
      // then we don't need a new id
      uint32_t id = ids++;
      {
        std::lock_guard _(cancelablesMut);
        // the first time we create a cancelable,
        // we assume its use count is 0
        cancelables[id] = 0;
      }
      return AutoCancel(std::move(cancel), id);
    }
  };
} // namespace smv::utils
