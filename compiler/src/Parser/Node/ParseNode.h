#pragma once

#include <Tokeniser/Token.h>

#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

namespace Cepheid::Parser::Nodes {
enum class NodeType {
  Module,
  Identifier,
  TypeName,
  Function,
  Scope,
  VariableDeclaration,
  ReturnType,
  ReturnStatement,
  Expression,
  BinaryOperation,
  UnaryOperation,
  Operator,
  IntegerLiteral,
  Conditional,
  Loop,
};

class Node;
using NodePtr = std::unique_ptr<Node>;

class Node {
 public:
  template <typename... ArgsT>
  static NodePtr make(ArgsT&&... args);

  explicit Node(NodeType type);
  Node(NodeType type, Tokens::Token token);
  Node(NodeType type, NodePtr child);
  virtual ~Node() = default;
  void addChild(NodePtr child);

  [[nodiscard]] NodeType type() const;
  [[nodiscard]] const std::optional<Tokens::Token>& token() const;

  [[nodiscard]] const std::vector<NodePtr>& children() const;

  [[nodiscard]] const Node* child(NodeType type) const;

 private:
  NodeType m_type;
  std::optional<Tokens::Token> m_token;
  std::vector<NodePtr> m_children;
};

template <typename... ArgsT>
inline NodePtr Node::make(ArgsT&&... args) {
  return std::make_unique<Node>(std::forward<ArgsT>(args)...);
}

}  // namespace Cepheid::Parser