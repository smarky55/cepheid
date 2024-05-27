#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Parser::Nodes {
class Scope;

class Function : public Node {
 public:
  explicit Function(std::string_view name);
  ~Function() override = default;

  [[nodiscard]] const std::string& name() const;

  void addParameter();

  void setReturnType(NodePtr returnType);
  [[nodiscard]] const Node* returnType() const;

  void setScope(std::unique_ptr<Scope> scope);
  [[nodiscard]] const Scope* scope() const;

  [[nodiscard]] size_t requiredStackSpace() const;

 private:
  std::string m_name;
  NodePtr m_returnType;
  std::unique_ptr<Scope> m_scope;
};
;
}  // namespace Cepheid::Parser::Nodes
