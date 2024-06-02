#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Parser::Nodes {
class Scope;

class Conditional : public Node {
 public:
  explicit Conditional(NodePtr expression, std::unique_ptr<Scope> scope);
  ~Conditional() override = default;

  [[nodiscard]] const Node* expression() const;

  [[nodiscard]] const Scope* scope() const;

 private:
  NodePtr m_expression;
  std::unique_ptr<Scope> m_scope;
};

}  // namespace Cepheid::Parser::Nodes
