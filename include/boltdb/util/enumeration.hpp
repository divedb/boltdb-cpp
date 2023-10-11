#ifndef BOLTDB_CPP_UTIL_ENUMERATION_HPP_
#define BOLTDB_CPP_UTIL_ENUMERATION_HPP_

#include <iterator>

namespace boltdb {

template <typename T>
constexpr bool is_container = requires(T v) {
  v.begin();
  v.end();
  v.size();
};

template <typename Iter>
class EnumIter {
 public:
  // TODO(gc): is it necessary to add reference type, change Iter during iteration?
  using value_type = std::pair<std::size_t, typename Iter::value_type>;

  EnumIter() = default;
  EnumIter(Iter first, std::size_t index = 0) : element_(first, index) {}

  EnumIter& operator++() {
    ++element_.first;
    ++element_.second;

    return *this;
  }

  EnumIter operator++(int) {
    EnumIter tmp(*this);

    element_.first++;
    element_.second++;

    return tmp;
  }

  value_type operator*() { return std::make_pair(element_.first, *element_.second); }

  // TODO(gc): swap?

  bool operator==(const EnumIter& other) const { return element_ == other.element_; }
  bool operator!=(const EnumIter& other) const { return !(*this == other); }

 private:
  std::pair<std::size_t, Iter> element_;
};

template <typename Container>
  requires is_container<Container>
class Enumeration {
 public:
  using iterator = decltype(declval<Container>().begin());
  using iterator_traits = std::iterator_traits<itertaor>;
  using value_type = typename iterator_traits::value_type;
  using reference = typename iterator_traits::reference;
  using pointer = typename iterator_traits::pointer;
  using iterator_category = typename iterator_traits::iterator_category;
  using difference_type = typename iterator_traits::difference_type;

  Enumeration(const Container& container) : container_(container) {}

  EnumIter begin() { return EnumIter{container_.begin()}; }
  EnumIter end() { return EnumIter{container_.end(), container_.size()}; }
  const EnumIter cbegin() const { return EnumIter{container_.begin()}; }
  const EnumIter cend() const { return EnumIter{container_.end(), container_.size()}; }

 private:
  Container& container_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_ENUMERATION_HPP_