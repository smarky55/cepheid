#pragma once
#include <string>

namespace Cepheid::Eval {
class Type {
 public:
  Type(std::string_view name, std::size_t size, std::size_t alignment);

  [[nodiscard]] std::string_view name() const;

  [[nodiscard]] std::size_t size() const;

  [[nodiscard]] std::size_t alignment() const;

 private:
  std::string m_name;
  std::size_t m_size;
  std::size_t m_alignment;
};

}  // namespace Cepheid::Eval
