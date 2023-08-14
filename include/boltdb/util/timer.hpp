#ifndef BOLTDB_CPP_UTIL_TIMER_HPP_
#define BOLTDB_CPP_UTIL_TIMER_HPP_

#include <chrono>

namespace boltdb {

// This is a basic timer.
class Timer {
 public:
  explicit Timer(double timeout_sec);

  // Get elaspsed time in second.
  [[nodiscard]] double elapsed_sec() const;

  // Get true if it's timeout now; otherwise false.
  [[nodiscard]] bool is_timeout() const;

 private:
  double timeout_;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_TIMER_HPP_
