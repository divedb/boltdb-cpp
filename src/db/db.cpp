#include "boltdb/db/db.hpp"

#include <fcntl.h>

namespace boltdb {

[[nodiscard]] Status open(std::string path, int mode, Options options,
                          DB** out_db) {
  int flag = O_RDWR;

  if (options.is_read_only()) {
    flag = O_RDONLY;
  }
}

}  // namespace boltdb