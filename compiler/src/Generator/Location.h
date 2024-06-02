#pragma once

#include <string>

namespace Cepheid::Gen {
/**
 * Represents a location for data such as a register or memory location
 */
class Location {
 public:
  virtual ~Location() = default;

  auto operator<=>(const Location&) const = default;
  bool operator==(const Location& other) const = default;

  /// Get this location as an assembly string
  [[nodiscard]] virtual std::string asAsm(size_t size) const = 0;
};

}  // namespace Cepheid::Gen
