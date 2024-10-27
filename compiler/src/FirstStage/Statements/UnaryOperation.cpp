#include "UnaryOperation.h"

#include <FirstStage/Statements/StatementVisitor.h>

Cepheid::Eval::UnaryOperation::UnaryOperation(
    OperationType type, std::shared_ptr<Value> result, std::shared_ptr<Value> operand)
    : m_type(type), m_result(std::move(result)), m_operand(std::move(operand)) {
}

void Cepheid::Eval::UnaryOperation::accept(StatementVisitor& visitor) {
  visitor.visit(*this);
}
