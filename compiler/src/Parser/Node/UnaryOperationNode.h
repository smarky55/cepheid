#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Parser {

enum class UnaryOperation { Negate, Not, Decrement, Increment };

[[nodiscard]] UnaryOperation tokenToUnaryOperation(const Tokens::Token& token);

class UnaryOperationNode : public Node {
public:
  explicit UnaryOperationNode(UnaryOperation operation);

  [[nodiscard]] UnaryOperation operation() const;

  void setOperand(NodePtr operand);
  [[nodiscard]] Node* operand() const;

private:
  UnaryOperation m_operation;
  NodePtr m_operand;
};

}  // namespace Cepheid::Parser
// Cepheid
