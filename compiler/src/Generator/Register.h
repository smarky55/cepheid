#pragma once

#include <Generator/Location.h>

namespace Cepheid::Gen {
class Register : public Location {
 public:
  enum class Kind {
    Original,
    AMD64,
    // TODO XMM registers
  };

  Register(Kind kind, std::string_view name);

  auto operator<=>(const Register&) const = default;
  bool operator==(const Register& other) const = default;

  [[nodiscard]] std::string asAsm(size_t size) const override;

 private:
  Kind m_kind;
  std::string m_name;
};

}  // namespace Cepheid::Gen
