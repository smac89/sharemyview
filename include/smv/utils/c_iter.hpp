#pragma once

#include <cstddef>
#include <iterator>

namespace {
  template<typename T>
  struct CPterIteratorSentinel
  {
    const T *const mRef;
    std::size_t    mIndex;

    const T                  &operator*() { return mRef[mIndex]; }
    CPterIteratorSentinel<T> &operator++()
    {
      ++mIndex;
      return *this;
    }
    bool operator!=(const CPterIteratorSentinel<T> &other) const
    {
      return mIndex != other.mIndex;
    }

    // This is used by fmt::join
    // see https://en.cppreference.com/w/cpp/named_req/ForwardIterator
    using value_type        = const T;
    using pointer           = const T *const;
    using reference         = const T &;
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
  };
} // namespace

namespace smv::utils {
  template<typename T>
  class CPtrIterator
  {
  public:
    explicit CPtrIterator(const T *const ptr, const std::size_t size)
      : mPtr(ptr)
      , mSize(size)
    {
    }

    CPterIteratorSentinel<T> begin() const { return { mPtr, 0 }; }
    CPterIteratorSentinel<T> end() const { return { nullptr, mSize }; }

  private:
    const T *const    mPtr;
    const std::size_t mSize;
  };
} // namespace smv::utils
