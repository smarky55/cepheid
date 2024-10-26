#include "VariableValue.h"

Cepheid::Eval::VariableValue::VariableValue(const Type* type, const std::string& name) : Value(type), m_name(name) {
}
