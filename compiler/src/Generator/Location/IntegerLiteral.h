#pragma once

#include <Generator/Location/Location.h>

namespace Cepheid::Gen {

class IntegerLiteral : public Location {
 public:
  explicit IntegerLiteral(std::string_view value);

  [[nodiscard]] std::string asAsm(size_t size) const override;

 private:
  std::string m_value;
};
}  // namespace Cepheid::Gen
