#include "FunctionNode.h"

#include <Parser/Node/ScopeNode.h>

Cepheid::Parser::FunctionNode::FunctionNode(std::string_view name) : Node(NodeType::Function), m_name(name) {
}

const std::string& Cepheid::Parser::FunctionNode::name() const {
  return m_name;
}

void Cepheid::Parser::FunctionNode::addParameter() {
  // TODO
}

void Cepheid::Parser::FunctionNode::setReturnType(NodePtr returnType) {
  m_returnType = std::move(returnType);
}

const Cepheid::Parser::Node* Cepheid::Parser::FunctionNode::returnType() const {
  return m_returnType.get();
}

void Cepheid::Parser::FunctionNode::setScope(std::unique_ptr<ScopeNode> scope) {
  m_scope = std::move(scope);
}

const Cepheid::Parser::ScopeNode* Cepheid::Parser::FunctionNode::scope() const {
  return m_scope.get();
}
