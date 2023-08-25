#include "boltdb/util/exception.hpp"

namespace boltdb {

std::string exception_type_to_string(ExceptionType type) {
  switch (type) {
    case ExceptionType::kInvalid:
      return "Invalid";
    case ExceptionType::kIO:
      return "IO";
    default:
      return "Unknown";
  }
}

Exception::Exception(std::string message)
    : std::exception(),
      type_(ExceptionType::kInvalid),
      message_(std::move(message)) {}

Exception::Exception(ExceptionType exception_type, std::string message)
    : std::exception(),
      type_(exception_type),
      message_(exception_type_to_string(type_) + ": " + message) {}

const char* Exception::what() const noexcept { return message_.c_str(); }

DBException::DBException(std::string error) : error_(std::move(error)) {}

const char* DBException::what() const noexcept { return error_.c_str(); }

}  // namespace boltdb