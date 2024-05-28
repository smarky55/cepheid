#include "Scope.h"

#include <Parser/Node/VariableDeclaration.h>

using namespace Cepheid::Parser::Nodes;

Scope::Scope() : Node(NodeType::Scope) {
}

void Scope::addStatement(NodePtr statement) {
  if (const auto* scopeNode = dynamic_cast<const Scope*>(statement.get())) {
    m_scopes.push_back(scopeNode);
  }
  if (const auto* variableNode = dynamic_cast<const VariableDeclaration*>(statement.get())) {
    m_locals.push_back(variableNode);
  }

  m_statements.push_back(std::move(statement));
}

const std::vector<NodePtr>& Scope::statements() const {
  return m_statements;
}

size_t Scope::requiredStackSpace() const {
  size_t localsSize = 0;
  for (const VariableDeclaration* local : m_locals) {
    localsSize += 8;  // TODO: Not everything is 8 bytes
  }

  size_t scopesMax = 0;
  for (const Scope* scope : m_scopes) {
    scopesMax = std::max(scopesMax, scope->requiredStackSpace());
  }

  return localsSize + scopesMax;
}
