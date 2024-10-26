#include "Value.h"

Cepheid::Eval::Value::Value(const Type* type) : m_type(type) {
}

const Cepheid::Eval::Type* Cepheid::Eval::Value::type() const {
  return m_type;
}
