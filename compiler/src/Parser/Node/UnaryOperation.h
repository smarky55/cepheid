#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Parser::Nodes {

enum class UnaryOperationType { Negate, Not, Decrement, Increment };

[[nodiscard]] UnaryOperationType tokenToUnaryOperation(const Tokens::Token& token);

class UnaryOperation : public Node {
 public:
  explicit UnaryOperation(UnaryOperationType operation);
  ~UnaryOperation() override = default;

  [[nodiscard]] UnaryOperationType operation() const;

  void setOperand(NodePtr operand);
  [[nodiscard]] const Node* operand() const;

 private:
  UnaryOperationType m_operation;
  NodePtr m_operand;
};

}  // namespace Cepheid::Parser
// Cepheid
