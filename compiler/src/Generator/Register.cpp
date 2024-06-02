#include "Register.h"

#include <Generator/GenerationException.h>

Cepheid::Gen::Register::Register(Kind kind, std::string_view name) : m_kind(kind), m_name(name) {
}

std::string Cepheid::Gen::Register::asAsm(size_t size) const {
  switch (m_kind) {
    case Kind::Original: {
      switch (size) {
        case 1:
          return m_name + "l";  // al
        case 2:
          return m_name + "x";  // ax
        case 4:
          return "e" + m_name + "x";  // eax
        case 8:
          return "r" + m_name + "x";  // rax
        default:
          throw GenerationException("Unexpected register size");
      }
    }
    case Kind::AMD64: {
      switch (size) {
        case 1:
          return m_name + "b";  // r8b
        case 2:
          return m_name + "w";  // r8w
        case 4:
          return m_name + "d";  // r8d
        case 8:
          return m_name;  // r8
        default:
          throw GenerationException("Unexpected register size");
      }
    }
    default:
      throw GenerationException("Unhandled register kind");
  }
}
