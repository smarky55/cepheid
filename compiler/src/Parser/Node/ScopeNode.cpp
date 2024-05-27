#include "ScopeNode.h"

using namespace Cepheid::Parser;

ScopeNode::ScopeNode() : Node(NodeType::Scope) {
}

void ScopeNode::addStatement(NodePtr statement) {
  if (const auto* scopeNode = dynamic_cast<const ScopeNode*>(statement.get())) {
    m_scopes.push_back(scopeNode);
  }

  m_statements.push_back(std::move(statement));
}

const std::vector<NodePtr>& ScopeNode::statements() const {
  return m_statements;
}
