#include "BinaryOperation.h"

#include <FirstStage/Statements/StatementVisitor.h>

Cepheid::Eval::BinaryOperation::BinaryOperation(
    Type type, std::shared_ptr<Value> result, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs)
    : m_type(type), m_result(std::move(result)), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)) {
}

void Cepheid::Eval::BinaryOperation::accept(StatementVisitor& visitor) {
  visitor.visit(*this);
}
