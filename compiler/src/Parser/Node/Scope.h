#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Parser::Nodes {
class VariableDeclaration;

class Scope : public Node {
 public:
  Scope();
  ~Scope() override = default;

  void addStatement(NodePtr statement);

  [[nodiscard]] const std::vector<NodePtr>& statements() const;

  [[nodiscard]] const std::vector<const VariableDeclaration*>& locals() const;

  [[nodiscard]] size_t requiredStackSpace() const;

 private:
  std::vector<NodePtr> m_statements;
  std::vector<const VariableDeclaration*> m_locals;
  std::vector<const Scope*> m_scopes;
};

}  // namespace Cepheid::Parser::Nodes
