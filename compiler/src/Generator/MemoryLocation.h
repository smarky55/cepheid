#pragma once

#include <Generator/Location.h>

namespace Cepheid::Gen {

class MemoryLocation : public Location {
 public:
  explicit MemoryLocation(std::string_view location);

  [[nodiscard]] std::string asAsm(size_t size) const override;

 private:
  std::string m_location;
};

}  // namespace Cepheid::Gen
