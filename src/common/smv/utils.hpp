#pragma once
#include <cstddef>

namespace {
template <typename T> struct CPterIteratorSentinel {
  T *const mRef;
  std::size_t mIndex;

  T &operator*() { return mRef[mIndex]; }
  CPterIteratorSentinel<T> &operator++() {
    ++mIndex;
    return *this;
  }
  bool operator!=(const CPterIteratorSentinel<T> &other) const {
    return mIndex != other.mIndex;
  }
};
} // namespace

namespace smv::utils {
template <typename T> class CPtrIterator {
public:
  explicit CPtrIterator(T *const ptr, const std::size_t size)
      : mPtr(ptr), mSize(size) {}

  CPterIteratorSentinel<T> begin() { return {mPtr, 0}; }
  CPterIteratorSentinel<T> end() { return {nullptr, mSize}; }

private:
  T *const mPtr;
  const std::size_t mSize;
};
} // namespace smv::utils
