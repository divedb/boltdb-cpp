#ifndef BOLTDB_CPP_MEMORY_POOL_HPP_
#define BOLTDB_CPP_MEMORY_POOL_HPP_

#include <cstddef>
#include <memory_resource>
#include <sstream>
#include <string>

#include "boltdb/alloc/memory_resource_tracker.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb {

// The MemoryPool class provides memory allocation and deallocation services.
// It utilizes the Singleton Pattern to ensure a single instance of the pool
// exists. The MemoryPool attempts to allocate small-sized blocks (< PageSize/4)
// to the stack and large-sized blocks to the heap. However, due to the upstream
// resource being 'new_delete_resource', small-sized blocks may also be
// allocated to the heap if the total size exceeds 1MB.
class MemoryPool {
 public:
  static constexpr int kPageSize = (1 << 10);        // 4096
  static constexpr int kInitBufferSize = (1 << 20);  // 1M

  // Get a singleton pool instance.
  static MemoryPool& instance() {
    static MemoryPool pool;

    return pool;
  }

  // Allocate a block of memory with the specified number of bytes.
  // Returns a pointer to the allocated memory block.
  // Note: the caller is responsible for managing the lifetime of the allocated
  // memory.
  Byte* allocate(std::size_t nbytes) {
    if (is_small_size(nbytes)) {
      return stack_pool_.allocate(nbytes);
    }

    return heap_pool_.allocate(nbytes);
  }

  // Deallocate a previously allocated memory block specified by the pointer 'p'
  // and the number of bytes 'nbytes'. It is essential to deallocate exactly the
  // same pointer and bytes returned by the corresponding `allocate` method.
  // Otherwise, an 'invalid_argument' exception will be thrown. Users are
  // responsible for ensuring proper deallocation to avoid memory-related
  // issues.
  void deallocate(Byte* p, std::size_t nbytes) {
    if (is_small_size(nbytes)) {
      stack_pool_.deallocate(p, nbytes);

      return;
    }

    heap_pool_.deallocate(p, nbytes);
  }

  // Get the number of bytes that haven't been deallocated.
  [[nodiscard]] std::size_t bytes_outstanding() const {
    return stack_tracker_.bytes_outstanding() +
           heap_tracker_.bytes_outstanding();
  }

  // Get statistic information, including number of bytes have been allocated
  // and deallocated, and high water bytes.
  [[nodiscard]] std::string statistic() const {
    std::ostringstream oss;

    oss << "=== stack allocation info ===\n";
    stack_tracker_.dump(oss);

    oss << "=== heap allocation info ===\n";
    heap_tracker_.dump(oss);

    return oss.str();
  }

 protected:
  MemoryPool()
      : buffer_(kInitBufferSize),
        stack_tracker_(&buffer_),
        stack_pool_(&stack_tracker_),
        heap_pool_(&heap_tracker_) {}

 private:
  static bool is_small_size(std::size_t nbytes) {
    return nbytes < (kPageSize / 4);
  }

  std::pmr::monotonic_buffer_resource buffer_;
  MemoryResourceTracker stack_tracker_;
  MemoryResourceTracker heap_tracker_;
  std::pmr::polymorphic_allocator<Byte> stack_pool_;
  std::pmr::polymorphic_allocator<Byte> heap_pool_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_MEMORY_POOL_HPP_
