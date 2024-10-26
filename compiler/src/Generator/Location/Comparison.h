#pragma once

#include <Generator/Location/Location.h>

namespace Cepheid::Gen {
class Comparison : public Location {
 public:
  enum class Type { Less, LessEqual, Equal, NotEqual, Greater, GreaterEqual };

  explicit Comparison(Type type);

  [[nodiscard]] std::string asAsm(size_t size) const override;
  [[nodiscard]] std::string setInstruction() const;
  [[nodiscard]] std::string jmpInstruction(bool inverse) const;

 private:
  Type m_type;
};

}  // namespace Cepheid::Gen
