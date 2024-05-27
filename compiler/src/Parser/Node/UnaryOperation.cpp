#include "UnaryOperation.h"

#include <map>

using namespace Cepheid::Parser::Nodes;

UnaryOperationType Cepheid::Parser::Nodes::tokenToUnaryOperation(const Cepheid::Tokens::Token& token) {
  static std::map<std::string_view, UnaryOperationType> tokenToOp = {
      {"-", UnaryOperationType::Negate},
      {"!", UnaryOperationType::Not},
      {"--", UnaryOperationType::Decrement},
      {"++", UnaryOperationType::Increment}};
  return tokenToOp.at(*token.value);
}

UnaryOperation::UnaryOperation(UnaryOperationType operation)
    : Node(NodeType::UnaryOperation), m_operation(operation) {
}

UnaryOperationType UnaryOperation::operation() const {
  return m_operation;
}

void UnaryOperation::setOperand(NodePtr operand) {
  m_operand = std::move(operand);
}

const Node* UnaryOperation::operand() const {
  return m_operand.get();
}