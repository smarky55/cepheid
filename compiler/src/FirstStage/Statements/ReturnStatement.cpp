#include "ReturnStatement.h"

#include <FirstStage/Statements/StatementVisitor.h>
#include <utility>

Cepheid::Eval::ReturnStatement::ReturnStatement(std::shared_ptr<Value> result) : m_result(std::move(result)) {
}

void Cepheid::Eval::ReturnStatement::accept(StatementVisitor& visitor) {
  visitor.visit(*this);
}
