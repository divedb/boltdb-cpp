#include "boltdb/util/exception.hpp"

using namespace boltdb;

DBException::DBException(const std::string& error) : _error(error) {}

const char* DBException::what() const noexcept { return _error.c_str(); }
