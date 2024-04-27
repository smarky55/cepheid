#pragma once

#include <exception>

namespace Cepheid::Parser {

class ParseException : public std::exception {
 public:
  using std::exception::exception;
};
}  // namespace Cepheid::Parser