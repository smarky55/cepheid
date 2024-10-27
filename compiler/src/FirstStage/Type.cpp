#include "Type.h"

using namespace Cepheid::Eval;

Type::Type(std::string_view name, std::size_t size, std::size_t alignment)
    : m_name(name), m_size(size), m_alignment(alignment) {
}

std::string_view Type::name() const {
  return m_name;
}

std::size_t Type::size() const {
  return m_size;
}

std::size_t Type::alignment() const {
  return m_alignment;
}
