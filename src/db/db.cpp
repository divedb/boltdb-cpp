#include "boltdb/db/db.hpp"

#include "boltdb/fs/file_system.hpp"

namespace boltdb {

[[nodiscard]] Status open(std::string path, int permission, Options options,
                          DB** out_db) {
  auto handle = FileSystem::open(path.c_str(), options.open_flag(), permission);

  if (handle == nullptr) {
    return Status(StatusType::kStatusErr, "Fail to open: " + path);
  }
}

}  // namespace boltdb