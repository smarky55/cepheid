#include "BinaryOperation.h"

#include <map>

using namespace Cepheid::Parser::Nodes;

BinaryOperationType Cepheid::Parser::Nodes::tokenToBinaryOperation(const Cepheid::Tokens::Token& token) {
  static std::map<std::string_view, BinaryOperationType> tokenToOp = {
      {"+", BinaryOperationType::Add},
      {"-", BinaryOperationType::Subtract},
      {"*", BinaryOperationType::Multiply},
      {"/", BinaryOperationType::Divide},
      {"<", BinaryOperationType::LessThan},
      {"<=", BinaryOperationType::LessEqual},
      {">", BinaryOperationType::GreaterThan},
      {">=", BinaryOperationType::GreaterEqual},
      {"==", BinaryOperationType::Equal},
      {"!=", BinaryOperationType::NotEqual},
      {"=", BinaryOperationType::Assign}};
  return tokenToOp.at(*token.value);
}

BinaryOperation::BinaryOperation(BinaryOperationType operation)
    : Node(NodeType::BinaryOperation), m_operation(operation) {
}
BinaryOperationType BinaryOperation::operation() const {
  return m_operation;
}

void BinaryOperation::setLHS(NodePtr lhsNode) {
  m_lhsNode = std::move(lhsNode);
}

const Node* BinaryOperation::lhs() const {
  return m_lhsNode.get();
}

void BinaryOperation::setRHS(NodePtr rhsNode) {
  m_rhsNode = std::move(rhsNode);
}

const Node* BinaryOperation::rhs() const {
  return m_rhsNode.get();
}