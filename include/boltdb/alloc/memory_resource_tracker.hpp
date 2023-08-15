#ifndef BOLTDB_CPP_MEMORY_RESOURCE_TRACKER_HPP_
#define BOLTDB_CPP_MEMORY_RESOURCE_TRACKER_HPP_

// https://github.com/phalpern/CppCon2017Code/blob/master/test_resource.cpp

#include <algorithm>
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory_resource>
#include <vector>

namespace boltdb {

// MemoryResourceTracker keeps track of the memory allocation and deallocation.
// The main purpose is to detect memory leaks.
class MemoryResourceTracker : public std::pmr::memory_resource {
 public:
  MemoryResourceTracker() = default;

  explicit MemoryResourceTracker(
      std::pmr::memory_resource* upstream)
      : upstream_(upstream)
         {}

  // Get the total number of bytes that have been allocated.
  [[nodiscard]] std::size_t bytes_allocated() const { return bytes_allocated_; }

  // Get the total number of bytes that have been deallocated.
  [[nodiscard]] std::size_t bytes_deallocated() const {
    return bytes_allocated_ - bytes_outstanding_;
  }

  // Get the number of bytes that haven't been deallocated.
  [[nodiscard]] std::size_t bytes_outstanding() const { return bytes_outstanding_; }

  // Get the highest number of allocated bytes.
  [[nodiscard]] std::size_t bytes_highwater() const { return bytes_highwater_; }

  std::ostream& dump(std::ostream& os) const {
    os << "[bytes allocated]:" << bytes_allocated() << '\n';
    os << "[bytes deallocated]:" << bytes_deallocated() << '\n';
    os << "[bytes high water]:" << bytes_highwater() << '\n';

    return os;
  }

 protected:
  void* do_allocate(std::size_t nbytes, std::size_t alignment) override {
    void* p = upstream_->allocate(nbytes, alignment);

    blocks_.push_back(Block{p, nbytes, alignment});
    bytes_allocated_ += nbytes;
    bytes_outstanding_ += nbytes;
    bytes_highwater_ = std::max(bytes_highwater_, bytes_outstanding_);

    return p;
  }

  void do_deallocate(void* p, std::size_t nbytes,
                     std::size_t alignment) override {
    // Check that deallocation args exactly match allocation args.
    // arguments. Note that, this check may not be necessary when this tracker
    // is used solely for internal purposes.
    auto i = std::find_if(blocks_.begin(), blocks_.end(),
                          [p](Block& block) { return block.ptr == p; });
    if (i == blocks_.end()) {
      throw std::invalid_argument("do_deallocate: Invalid pointer.");
    }

    if (i->nbytes != nbytes) {
      throw std::invalid_argument("do_deallocate: Size mismatch.");
    }

    if (i->alignment != alignment) {
      throw std::invalid_argument("do_deallocate: Alignment mismatch.");
    }

    upstream_->deallocate(p, i->nbytes, i->alignment);
    blocks_.erase(i);
    bytes_outstanding_ -= nbytes;
  }

  [[nodiscard]] bool do_is_equal(
      const std::pmr::memory_resource& other) const noexcept override {
    return this == &other;
  }

 private:
  struct Block {
    void* ptr;
    std::size_t nbytes;
    std::size_t alignment;
  };

  std::pmr::memory_resource* upstream_{std::pmr::get_default_resource()};
  std::pmr::vector<Block> blocks_;
  std::size_t bytes_allocated_{0};
  std::size_t bytes_outstanding_{0};
  std::size_t bytes_highwater_{0};
};

}  // namespace boltdb
#endif  // BOLTDB_CPP_MEMORY_RESOURCE_TRACKER_HPP_
