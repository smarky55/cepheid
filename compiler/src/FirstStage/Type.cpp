#include "Type.h"

using namespace Cepheid::Eval;

Type::Type(std::string_view name, std::size_t size) : m_name(name), m_size(size) {
}

std::string_view Type::name() const {
  return m_name;
}

std::size_t Type::size() const {
  return m_size;
}
