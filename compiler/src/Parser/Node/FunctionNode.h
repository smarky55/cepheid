#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Parser {
class ScopeNode;
}

namespace Cepheid::Parser {
class FunctionNode : public Node {
 public:
  explicit FunctionNode(std::string_view name);
  ~FunctionNode() override = default;

  [[nodiscard]] const std::string& name() const;

  void addParameter();

  void setReturnType(NodePtr returnType);
  [[nodiscard]] const Node* returnType() const;

  void setScope(std::unique_ptr<ScopeNode> scope);
  [[nodiscard]] const ScopeNode* scope() const;

 private:
  std::string m_name;
  NodePtr m_returnType;
  std::unique_ptr<ScopeNode> m_scope;
};
;
}  // namespace Cepheid::Parser
