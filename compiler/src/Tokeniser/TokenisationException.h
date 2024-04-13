#pragma once

#include <exception>

class TokenisationException : public std::exception {
  using std::exception::exception;
};
