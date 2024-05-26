#include "BinaryOperationNode.h"

#include <map>

using namespace Cepheid::Parser;

BinaryOperation Cepheid::Parser::tokenToBinaryOperation(const Cepheid::Tokens::Token& token) {
  static std::map<std::string_view, BinaryOperation> tokenToOp = {
      {"+", BinaryOperation::Add},
      {"-", BinaryOperation::Subtract},
      {"*", BinaryOperation::Multiply},
      {"/", BinaryOperation::Divide},
      {"<", BinaryOperation::LessThan},
      {"<=", BinaryOperation::LessEqual},
      {">", BinaryOperation::GreaterThan},
      {">=", BinaryOperation::GreaterEqual},
      {"==", BinaryOperation::Equal},
      {"!=", BinaryOperation::NotEqual}};
  return tokenToOp.at(*token.value);
}

BinaryOperationNode::BinaryOperationNode(BinaryOperation operation)
    : Node(NodeType::BinaryOperation), m_operation(operation) {
}
BinaryOperation BinaryOperationNode::operation() const {
  return m_operation;
}

void BinaryOperationNode::setLHS(NodePtr lhsNode) {
  m_lhsNode = std::move(lhsNode);
}

Node* BinaryOperationNode::lhs() const {
  return m_lhsNode.get();
}

void BinaryOperationNode::setRHS(NodePtr rhsNode) {
  m_rhsNode = std::move(rhsNode);
}

Node* BinaryOperationNode::rhs() const {
  return m_rhsNode.get();
}