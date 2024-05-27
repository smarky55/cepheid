#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Parser::Nodes {
class VariableDeclaration : public Node {
 public:
  VariableDeclaration(NodePtr typeName, std::string_view name);
  ~VariableDeclaration() override = default;

  [[nodiscard]] const Node* typeName() const;
  [[nodiscard]] const std::string& name() const;

 private:
  NodePtr m_typeName;
  std::string m_name;
};

}  // namespace Cepheid::Parser::Nodes
