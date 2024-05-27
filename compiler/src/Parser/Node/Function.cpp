#include "Function.h"

#include <Parser/Node/Scope.h>

using namespace Cepheid::Parser::Nodes;

Function::Function(std::string_view name) : Node(NodeType::Function), m_name(name) {
}

const std::string& Function::name() const {
  return m_name;
}

void Function::addParameter() {
  // TODO
}

void Function::setReturnType(NodePtr returnType) {
  m_returnType = std::move(returnType);
}

const Node* Function::returnType() const {
  return m_returnType.get();
}

void Function::setScope(std::unique_ptr<Scope> scope) {
  m_scope = std::move(scope);
}

const Scope* Function::scope() const {
  return m_scope.get();
}

size_t Function::requiredStackSpace() const {
  // TODO: Add up stack space for parameters
  if (!m_scope) {
    return 0;
  }
  return m_scope->requiredStackSpace();
}
