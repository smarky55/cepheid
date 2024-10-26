#pragma once
#include <FirstStage/Statements/Statement.h>
#include <Parser/Node/UnaryOperation.h>

namespace Cepheid::Eval {
class Value;

class UnaryOperation : public Statement {
 public:
  using OperationType = Parser::Nodes::UnaryOperationType;
  UnaryOperation(OperationType type, std::shared_ptr<Value> result, std::shared_ptr<Value> operand);

 private:
  OperationType m_type;
  std::shared_ptr<Value> m_result;
  std::shared_ptr<Value> m_operand;
};

}  // namespace Cepheid::Eval
