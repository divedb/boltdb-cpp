#ifndef BOLTDB_CPP_UTIL_STATUS_HPP_
#define BOLTDB_CPP_UTIL_STATUS_HPP_

#include <string>

#include "boltdb/util/types.hpp"

namespace boltdb {

enum StatusType : i8 { kStatusOK, kStatusErr, kStatusCorrupt };

class Status {
 public:
  Status() : status_type_(StatusType::kStatusOK) {}
  Status(StatusType status_type, std::string message)
      : status_type_(status_type), message_(std::move(message)) {}

  // Explicit convert to bool.
  // Get true if the status type is `kStatusOK`, otherwise false.
  explicit operator bool() const { return status_type_ == kStatusOK; }

  [[nodiscard]] StatusType status_type() const { return status_type_; }

 private:
  StatusType status_type_;
  std::string message_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_STATUS_HPP_
