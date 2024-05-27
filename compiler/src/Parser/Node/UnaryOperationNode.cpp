#include "UnaryOperationNode.h"

#include <map>

using namespace Cepheid::Parser;

UnaryOperation Cepheid::Parser::tokenToUnaryOperation(const Cepheid::Tokens::Token& token) {
  static std::map<std::string_view, UnaryOperation> tokenToOp = {
      {"-", UnaryOperation::Negate},
      {"!", UnaryOperation::Not},
      {"--", UnaryOperation::Decrement},
      {"++", UnaryOperation::Increment}};
  return tokenToOp.at(*token.value);
}

UnaryOperationNode::UnaryOperationNode(UnaryOperation operation)
    : Node(NodeType::UnaryOperation), m_operation(operation) {
}

UnaryOperation UnaryOperationNode::operation() const {
  return m_operation;
}

void UnaryOperationNode::setOperand(NodePtr operand) {
  m_operand = std::move(operand);
}

const Node* UnaryOperationNode::operand() const {
  return m_operand.get();
}