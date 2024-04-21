#pragma once

#include <exception>

namespace Cepheid::Tokens {
class TokenisationException : public std::exception {
  using std::exception::exception;
};
}  // namespace Cepheid::Token
