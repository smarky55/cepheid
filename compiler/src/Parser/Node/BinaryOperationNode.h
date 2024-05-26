#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Tokens {
struct Token;
}

namespace Cepheid::Parser {

enum class BinaryOperation {
  Add,
  Subtract,
  Multiply,
  Divide,
  LessThan,
  LessEqual,
  GreaterThan,
  GreaterEqual,
  Equal,
  NotEqual
};

[[nodiscard]] BinaryOperation tokenToOperation(const Tokens::Token& token);

class BinaryOperationNode : public Node {
 public:
  explicit BinaryOperationNode(BinaryOperation operation);

  [[nodiscard]] BinaryOperation operation() const;

  void setLHS(NodePtr lhsNode);
  [[nodiscard]] Node* lhs() const;

  void setRHS(NodePtr rhsNode);
  [[nodiscard]] Node* rhs() const;

 private:
  BinaryOperation m_operation;
  NodePtr m_lhsNode;
  NodePtr m_rhsNode;
};

}  // namespace Cepheid::Parser
