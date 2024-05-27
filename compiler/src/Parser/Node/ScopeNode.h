#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Parser {

class ScopeNode : public Node {
 public:
  ScopeNode();
  ~ScopeNode() override = default;

  void addStatement(NodePtr statement);

  [[nodiscard]] const std::vector<NodePtr>& statements() const;

 private:
  std::vector<NodePtr> m_statements;
  std::vector<const Node*> m_locals;
  std::vector<const ScopeNode*> m_scopes;
};

}  // namespace Cepheid::Parser
