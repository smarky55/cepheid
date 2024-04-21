#pragma once

#include <string>

namespace Cepheid {
class Compiler {
 public:
  std::string compile(std::string_view src) const;
};
}  // namespace Cepheid
