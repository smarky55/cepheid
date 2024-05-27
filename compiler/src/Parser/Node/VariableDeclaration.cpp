#include "VariableDeclaration.h"

Cepheid::Parser::Nodes::VariableDeclaration::VariableDeclaration(NodePtr typeName, std::string_view name)
    : Node(NodeType::VariableDeclaration), m_typeName(std::move(typeName)), m_name(name) {
}

const Cepheid::Parser::Nodes::Node* Cepheid::Parser::Nodes::VariableDeclaration::typeName() const {
  return m_typeName.get();
}

const std::string& Cepheid::Parser::Nodes::VariableDeclaration::name() const {
  return m_name;
}
