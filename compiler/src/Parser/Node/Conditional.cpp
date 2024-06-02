#include "Conditional.h"

#include <Parser/Node/Scope.h>

using namespace Cepheid::Parser::Nodes;

Conditional::Conditional(NodePtr expression, std::unique_ptr<Scope> scope)
    : Node(NodeType::Conditional), m_expression(std::move(expression)), m_scope(std::move(scope)) {
}

const Node* Conditional::expression() const {
  return m_expression.get();
}

const Scope* Conditional::scope() const {
  return m_scope.get();
}
