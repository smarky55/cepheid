#include "Loop.h"

#include <Parser/Node/Scope.h>

using namespace Cepheid::Parser::Nodes;

Loop::Loop(NodePtr initExpression, NodePtr conditionExpression, NodePtr updateExpression, std::unique_ptr<Scope> scope)
    : Node(NodeType::Loop),
      m_initExpression(std::move(initExpression)),
      m_conditionExpression(std::move(conditionExpression)),
      m_updateExpression(std::move(updateExpression)),
      m_scope(std::move(scope)) {
}

const Node* Loop::initExpression() const {
  return m_initExpression.get();
}

const Node* Loop::conditionExpression() const {
  return m_conditionExpression.get();
}

const Node* Loop::updateExpression() const {
  return m_updateExpression.get();
}

const Scope* Loop::scope() const {
  return m_scope.get();
}
