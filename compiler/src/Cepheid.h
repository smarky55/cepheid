#pragma once

#include <string>

class Cepheid {
 public:
  std::string compile(std::string_view src) const;
};
