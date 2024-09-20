#pragma once
#include <string>

namespace Cepheid::Eval {
class Type {
 public:
  Type(std::string_view name, std::size_t size);

  [[nodiscard]] std::string_view name() const;

  [[nodiscard]] std::size_t size() const;

 private:
  std::string m_name;
  std::size_t m_size;
};

}  // namespace Cepheid::Eval
