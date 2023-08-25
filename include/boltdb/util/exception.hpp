#ifndef BOLTDB_CPP_UTIL_EXCEPTION_HPP_
#define BOLTDB_CPP_UTIL_EXCEPTION_HPP_

#include <cstdarg>
#include <exception>
#include <string>

namespace boltdb {

//===--------------------------------------------------------------------===//
// Exception Types
//===--------------------------------------------------------------------===//
enum class ExceptionType { kInvalid = 0, kIO = 1 };

std::string exception_type_to_string(ExceptionType type);

class Exception : public std::exception {
 public:
  Exception(std::string message);
  Exception(ExceptionType exception_type, std::string message);

  const char* what() const noexcept override;

 private:
  ExceptionType type_;
  std::string message_;
};

class IOException : public Exception {
 public:
  IOException(std::string message) : Exception(ExceptionType::kIO, message) {}
};

class DBException : public std::exception {
 public:
  explicit DBException(std::string error);

  [[nodiscard]] const char* what() const noexcept override;

 private:
  std::string error_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_EXCEPTION_HPP_