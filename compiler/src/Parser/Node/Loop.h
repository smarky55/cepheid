#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Parser::Nodes {
class Scope;

class Loop : public Node {
 public:
  Loop(NodePtr initExpression, NodePtr conditionExpression, NodePtr updateExpression, std::unique_ptr<Scope> scope);
  ~Loop() override = default;

  [[nodiscard]] const Node* initExpression() const;
  [[nodiscard]] const Node* conditionExpression() const;
  [[nodiscard]] const Node* updateExpression() const;
  [[nodiscard]] const Scope* scope() const;

 private:
  NodePtr m_initExpression;
  NodePtr m_conditionExpression;
  NodePtr m_updateExpression;
  std::unique_ptr<Scope> m_scope;
};

}  // namespace Cepheid::Parser::Nodes
