#include "JumpStatement.h"

#include <FirstStage/Statements/StatementVisitor.h>

Cepheid::Eval::JumpStatement::JumpStatement(
    std::string_view destination, JumpIf jumpIf, std::shared_ptr<Value> comparisonValue)
    : m_destination(destination), m_comparison(std::move(comparisonValue)), m_jumpIf(jumpIf) {
}

void Cepheid::Eval::JumpStatement::accept(StatementVisitor& visitor) {
  visitor.visit(*this);
}
