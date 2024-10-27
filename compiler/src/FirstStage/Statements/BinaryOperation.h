#pragma once

#include <FirstStage/Statements/Statement.h>
#include <Parser/Node/BinaryOperation.h>

namespace Cepheid::Eval {
class Value;

class BinaryOperation : public Statement {
 public:
  using Type = Parser::Nodes::BinaryOperationType;
  BinaryOperation(Type type, std::shared_ptr<Value> result, std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs);

  void accept(StatementVisitor& visitor) override;

 private:
  Type m_type;
  std::shared_ptr<Value> m_result;
  std::shared_ptr<Value> m_lhs;
  std::shared_ptr<Value> m_rhs;
};

}  // namespace Cepheid::Eval
