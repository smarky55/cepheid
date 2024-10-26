#pragma once

#include <exception>

namespace Cepheid::Eval {
class EvaluationException : public std::exception {
 public:
  using std::exception::exception;
};

}  // namespace Cepheid::Eval
