#ifndef BOLTDB_CPP_UTIL_EXCEPTION_HPP_
#define BOLTDB_CPP_UTIL_EXCEPTION_HPP_

#include <exception>
#include <string>

namespace boltdb {

class DBException : public std::exception {
 public:
  DBException(const std::string& error);

  virtual const char* what() const noexcept override;

 private:
  std::string _error;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_EXCEPTION_HPP_