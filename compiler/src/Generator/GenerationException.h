#pragma once

#include <exception>

namespace Cepheid::Gen {

class GenerationException : public std::exception {
 public:
  using std::exception::exception;
};

}  // namespace Cepheid::Gen
