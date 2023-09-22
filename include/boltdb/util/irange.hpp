#ifndef BOLTDB_CPP_UTIL_IRANGE_HPP_
#define BOLTDB_CPP_UTIL_IRANGE_HPP_

#include <cstddef>
#include <iostream>
#include <iterator>
#include <type_traits>

namespace boltdb {

template <typename T>
  requires std::is_integral_v<T>
class IRangeIter;

template <typename T>
  requires std::is_integral_v<T>
class IRange {
 public:
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using size_type = std::size_t;

  // Construct an integer range [first, last) with the specified step.
  IRange(value_type first, value_type last, difference_type step)
      : first_(first), last_(last), step_(step) {}

  IRangeIter<T> begin() { return {first_, step_}; }
  IRangeIter<T> end() { return {first_ + step_ * size(), step_}; }

  IRangeIter<T> cbegin() const { return {first_, step_}; }
  IRangeIter<T> cend() const { return {first_ + step_ * size(), step_}; }

  size_type size() const {
    int diff = last_ - first_;
    int n = diff / step_;

    if (diff % step_ != 0) {
      n++;
    }

    return n;
  }

 private:
  int sign() const { return step_ < 0 ? -1 : 1; }

  value_type first_;
  value_type last_;
  difference_type step_;
};

template <typename T>
  requires std::is_integral_v<T>
class IRangeIter {
 public:
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using reference = T&;
  using iterator_category = std::random_access_iterator_tag;

  IRangeIter(value_type first, difference_type step)
      : first_(first), step_(step) {}

  IRangeIter& operator++() noexcept {
    first_ += step_;

    return *this;
  }

  IRangeIter operator++(int) noexcept {
    IRangeIter tmp(*this);
    ++(*this);

    return tmp;
  }

  IRangeIter& operator--() noexcept {
    first_ -= step_;

    return *this;
  }

  IRangeIter operator--(int) noexcept {
    IRangeIter tmp(*this);
    --(*this);

    return tmp;
  }

  IRangeIter operator+(difference_type n) const noexcept {
    IRangeIter tmp(*this);
    tmp += n;

    return tmp;
  }

  IRangeIter& operator+=(difference_type n) noexcept {
    first_ += n * step_;

    return *this;
  }

  IRangeIter operator-(difference_type n) const noexcept {
    return *this + (-n);
  }

  IRangeIter operator-=(difference_type n) noexcept {
    *this += -n;

    return *this;
  }

  reference operator*() { return first_; }

  template <typename U>
  friend bool operator==(const IRangeIter<U>& lhs, const IRangeIter<U>& rhs);

  template <typename U>
  friend bool operator!=(const IRangeIter<U>& lhs, const IRangeIter<U>& rhs);

  template <typename U>
  friend std::ptrdiff_t operator-(const IRangeIter<U>& lhs,
                                  const IRangeIter<U>& rhs);

 private:
  value_type first_;
  difference_type step_;
};

template <typename T>
inline bool operator==(const IRangeIter<T>& lhs, const IRangeIter<T>& rhs) {
  return lhs.first_ == rhs.first_ && lhs.step_ == rhs.step_;
}

template <typename T>
inline bool operator!=(const IRangeIter<T>& lhs, const IRangeIter<T>& rhs) {
  return !(lhs == rhs);
}

template <typename T>
inline std::ptrdiff_t operator-(const IRangeIter<T>& lhs,
                                const IRangeIter<T>& rhs) {
  return (lhs.first_ - rhs.first_) / rhs.step_;
}

template <typename T>
  requires std::is_integral_v<T>
IRange<T> make_irange(T first, T last, std::ptrdiff_t step = 1) {
  return {first, last, step};
}

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_IRANGE_HPP_