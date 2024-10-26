#include "JumpStatement.h"

Cepheid::Eval::JumpStatement::JumpStatement(
    std::string_view destination, JumpIf jumpIf, std::shared_ptr<Value> comparisonValue)
    : m_destination(destination), m_comparison(std::move(comparisonValue)), m_jumpIf(jumpIf) {
}
