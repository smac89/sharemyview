#pragma once

#include <cstddef>
#include <iterator>

namespace {
  template<typename T>
  struct IteratorSentinel
  {
    T *const    mRef;
    std::size_t mIndex;

    auto operator*() -> T & { return mRef[mIndex]; }
    auto operator--() -> IteratorSentinel<T> &
    {
      --mIndex;
      return *this;
    }
    auto operator++() -> IteratorSentinel<T> &
    {
      ++mIndex;
      return *this;
    }
    auto operator!=(const IteratorSentinel<T> &other) const -> bool
    {
      return mIndex != other.mIndex;
    }

    // This is used by fmt::join
    // see https://en.cppreference.com/w/cpp/named_req/ForwardIterator
    using value_type        = T;
    using pointer           = T *const;
    using reference         = T &;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
  };
} // namespace

// TODO: Replace this with Boost.Iterator
namespace smv::utils {
  template<typename T>
  class PtrIterator
  {
  public:
    explicit PtrIterator(T *ptr, const std::size_t size)
      : mPtr(ptr)
      , mSize(size)
    {
    }

    auto begin() const -> IteratorSentinel<T> { return { mPtr, 0 }; }
    auto end() const -> IteratorSentinel<T> { return { nullptr, mSize }; }

  private:
    T *const          mPtr;
    const std::size_t mSize;
  };

  template<typename T>
  struct CPtrIterator: PtrIterator<const T>
  {
    using PtrIterator<const T>::PtrIterator;
    using PtrIterator<const T>::begin;
    using PtrIterator<const T>::end;
  };
} // namespace smv::utils
