#ifndef BOLTDB_CPP_UTIL_MEMORY_MAP_HPP_
#define BOLTDB_CPP_UTIL_MEMORY_MAP_HPP_

namespace boltdb {

// mmap() creates a new mapping in the virtual address space of the calling process.
// The starting address for the new mapping is specified in `addr`.
// The `length` argument specifies the length of the mapping (which must be greater than 0).
// If `addr` is NULL, then the kernel chooses the (page-aligned) address at which to
// create the mapping.
// If `addr` is not NULL, then the kernel takes it as a hint about where to place the mapping;
// on Linux, the kernel will pick a nerby page boundary (but always above or equal to the value
// specified by /proc/sys/vm/mmap_min_addr) and attempt to create the mmaping there.
// If another mapping already exists there, the kernel picks a new address
// that may or may not depend on the hint. The address of the new mapping is returned as the result
// of the call.
//
// The contents of a file mapping (as opposed to anonymous mapping; see MAP_ANONYMOUS below),
// are initialized using `length` bytes starting at offset `offset` in the file (or other object)
// referred to by the file descriptor `fd`. `offset` must be a multiple of the page size as returned
// by `sysconf(_SC_PAGE_SIZE)`.
//
// After the `mmap()` call has returned, the file descriptor, `fd`, can be closed immediately without
// invalidating the mapping.
class MemoryMap {};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_MEMORY_MAP_HPP_