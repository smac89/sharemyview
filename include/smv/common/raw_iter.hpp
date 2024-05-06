#pragma once

#include <cstddef>
#include <iterator>

namespace {
  template<typename T>
  struct IteratorSentinel
  {
    // This is used by fmt::join
    // see https://en.cppreference.com/w/cpp/named_req/ForwardIterator
    using value_type        = T;
    using pointer           = T *;
    using reference         = T &;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;

    auto operator*() -> reference { return mRef[mIndex]; }
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

    T          *mRef;
    std::size_t mIndex;
  };
} // namespace

// TODO: Replace this with Boost.Iterator
namespace smv::utils {
  template<typename T>
  class RawIterator
  {
  public:
    using iterator       = IteratorSentinel<T>;
    using const_iterator = IteratorSentinel<const T>;

    explicit RawIterator(T *ptr, const std::size_t size)
      : mPtr(ptr)
      , mSize(size)
    {
    }

    constexpr auto data() const -> T * { return mPtr; }
    constexpr auto size() const -> std::size_t { return mSize; }

    auto begin() const -> iterator { return { mPtr, 0 }; }
    auto end() const -> iterator { return { nullptr, mSize }; }
    auto cbegin() const -> const_iterator { return { mPtr, mSize }; }
    auto cend() const -> const_iterator { return { nullptr, mSize }; }
    auto rbegin() const -> iterator { return { mPtr, mSize - 1 }; }
    auto rend() const -> iterator { return { nullptr, 0 }; }
    auto crbegin() const -> const_iterator { return { mPtr, mSize - 1 }; }
    auto crend() const -> const_iterator { return { nullptr, 0 }; }

  private:
    T          *mPtr;
    std::size_t mSize;
  };
} // namespace smv::utils
