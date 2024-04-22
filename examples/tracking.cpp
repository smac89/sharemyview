#include <atomic>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <utility>

#include <spdlog/spdlog.h>

using Cancel = std::function<void()>;

class AutoCancel
{
  explicit AutoCancel(Cancel cancel, uint32_t id)
    : mCancel(std::move(cancel))
    , mId(id)
  {
    spdlog::info("created id: {}", id);
  }

public:
  AutoCancel(const AutoCancel &other)
    : mCancel(other.mCancel)
    , mId(other.mId)
  {
    std::lock_guard lk(cancelablesMut);
    if (auto ref = cancelables.find(other.mId); ref != cancelables.end()) {
      ref->second++;
      spdlog::info("Copy created via constructor id: {}", mId);
    }
  }
  AutoCancel &operator=(const AutoCancel &other)
  {
    if (mId != other.mId) {
      this->tryCancel();
      {
        std::lock_guard lk(cancelablesMut);
        if (auto ref = cancelables.find(other.mId); ref != cancelables.end()) {
          ref->second++;
        }
      }
      this->mCancel = other.mCancel;
      this->mId     = other.mId;
      spdlog::info("Copy created via assignment id: {}", mId);
    }
    return *this;
  }
  AutoCancel(AutoCancel &&other) = default;
  auto operator=(AutoCancel &&other) noexcept -> AutoCancel &
  {
    this->tryCancel();
    this->mCancel = std::move(other.mCancel);
    this->mId     = other.mId;
    spdlog::info("Move assignment id: {}", mId);
    return *this;
  }
  auto id() const -> uint32_t { return mId; }
  ~AutoCancel() { tryCancel(true); }
  void operator()()
  {
    std::lock_guard lk(cancelablesMut);
    if (hasCancel()) {
      doCancel();
    }
  }

protected:
  void doCancel(bool isDestroy = false)
  {
    spdlog::info("destroy id: {}", mId);

    mCancel();
    mCancel = nullptr;
    if (!isDestroy) {
      spdlog::info("untracking id: {}", mId);
      cancelables.erase(mId);
    }
    spdlog::info("cancelled id: {}", mId);
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

    assert(("No refs", cancelables.at(mId) == 0));
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
  auto hasCancel() const -> bool { return mCancel != nullptr; }

private:
  Cancel                                               mCancel;
  uint32_t                                             mId;
  inline static std::atomic_uint32_t                   ids { 1 };
  inline static std::unordered_map<uint32_t, uint32_t> cancelables;
  inline static std::mutex                             cancelablesMut;

public:
  static auto wrap(Cancel cancel) -> AutoCancel
  {
    uint32_t        id = ids++;
    std::lock_guard _(cancelablesMut);
    cancelables[id] = 0;
    return AutoCancel(std::move(cancel), id);
  }
};

auto createCancel(uint32_t id)
{
  auto f = [id]() {
    std::cout << "cancelling " << id << '\n';
  };
  return AutoCancel::wrap(f);
}

auto main() -> int
{
  AutoCancel track1(createCancel(1001));
  auto       track2 = createCancel(2002);
  track2            = std::move(track1);
  track2            = createCancel(3003);
  AutoCancel track3 = track2;
  track2();
  AutoCancel track4 = track3;
  AutoCancel track5 = track1;
  track5();
  return 0;
}
