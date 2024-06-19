#include "IntegerLiteral.h"

Cepheid::Gen::IntegerLiteral::IntegerLiteral(std::string_view value) : m_value(value){
}

std::string Cepheid::Gen::IntegerLiteral::asAsm(size_t size) const {
  return m_value;
}
