#pragma once

#include <cstddef>
#include <iterator>

namespace {
  template<typename T>
  struct CPterIteratorSentinel
  {
    T          *mRef;
    std::size_t mIndex;

    auto operator*() -> const T & { return mRef[mIndex]; }
    auto operator++() -> CPterIteratorSentinel<T> &
    {
      ++mIndex;
      return *this;
    }
    auto operator!=(const CPterIteratorSentinel<T> &other) const -> bool
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

// TODO: Replace this with Boost.Iterator
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

    auto begin() const -> CPterIteratorSentinel<T> { return { mPtr, 0 }; }
    auto end() const -> CPterIteratorSentinel<T> { return { nullptr, mSize }; }

  private:
    T          *mPtr;
    std::size_t mSize;
  };
} // namespace smv::utils
