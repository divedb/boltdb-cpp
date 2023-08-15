#include "boltdb/util/exception.hpp"

namespace boltdb {

  DBException::DBException(std::string error) : error_(std::move(error)) {}

  const char* DBException::what() const noexcept { return error_.c_str(); }

}