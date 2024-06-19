#include "Comparison.h"

#include <Generator/GenerationException.h>

Cepheid::Gen::Comparison::Comparison(Type type) : m_type(type) {
}

std::string Cepheid::Gen::Comparison::asAsm(size_t size) const {
  throw GenerationException("Internal compiler error");
}

std::string Cepheid::Gen::Comparison::setInstruction() const {
  switch (m_type) {
    case Type::Less:
      return "setl";
    case Type::LessEqual:
      return "setle";
    case Type::Equal:
      return "sete";
    case Type::NotEqual:
      return "setne";
    case Type::Greater:
      return "setg";
    case Type::GreaterEqual:
      return "setge";
  }
  throw GenerationException("Unhandled comparison type in set");
}

std::string Cepheid::Gen::Comparison::jmpInstruction(bool inverse) const {
  switch (m_type) {
    case Type::Less:
      return inverse ? "jnl" : "jl";
    case Type::LessEqual:
      return inverse ? "jnle" : "jle";
    case Type::Equal:
      return inverse ? "jne" : "je";
    case Type::NotEqual:
      return inverse ? "je" : "jne";
    case Type::Greater:
      return inverse ? "jng" : "jg";
    case Type::GreaterEqual:
      return inverse ? "jnge" : "jge";
  }
  throw GenerationException("Unhandled comparison type in jump");
}
