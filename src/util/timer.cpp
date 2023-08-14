#include "boltdb/util/timer.hpp"

namespace boltdb {

Timer::Timer(double timeout_sec)
    : timeout_(timeout_sec),
      start_(std::chrono::high_resolution_clock::now()) {}

double Timer::elapsed_sec() const {
  auto now = std::chrono::high_resolution_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::duration<double>>(now - start_)
          .count();

  return elapsed;
}

bool Timer::is_timeout() const {
  auto elapsed = elapsed_sec();

  return elapsed >= timeout_;
}

}  // namespace boltdb