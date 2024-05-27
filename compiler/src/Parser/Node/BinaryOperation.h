#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Parser::Nodes {

enum class BinaryOperationType {
  Add,
  Subtract,
  Multiply,
  Divide,
  LessThan,
  LessEqual,
  GreaterThan,
  GreaterEqual,
  Equal,
  NotEqual,
  Assign
};

[[nodiscard]] BinaryOperationType tokenToBinaryOperation(const Tokens::Token& token);

class BinaryOperation : public Node {
 public:
  explicit BinaryOperation(BinaryOperationType operation);
  ~BinaryOperation() override = default;

  [[nodiscard]] BinaryOperationType operation() const;

  void setLHS(NodePtr lhsNode);
  [[nodiscard]] const Node* lhs() const;

  void setRHS(NodePtr rhsNode);
  [[nodiscard]] const Node* rhs() const;

 private:
  BinaryOperationType m_operation;
  NodePtr m_lhsNode;
  NodePtr m_rhsNode;
};

}  // namespace Cepheid::Parser::Nodes
