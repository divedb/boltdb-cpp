#ifndef BOLTDB_CPP_UTIL_EXCEPTION_HPP_
#define BOLTDB_CPP_UTIL_EXCEPTION_HPP_

#include <exception>
#include <string>

namespace boltdb {

class DBException : public std::exception {
 public:
  explicit DBException(std::string error);

  [[nodiscard]] const char* what() const noexcept override;

 private:
  std::string error_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_EXCEPTION_HPP_