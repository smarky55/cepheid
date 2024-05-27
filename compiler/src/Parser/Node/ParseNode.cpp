#include "ParseNode.h"

using namespace Cepheid::Parser::Nodes ;

using Cepheid::Tokens::Token;

Node::Node(NodeType type) : m_type(type) {
}

Node::Node(NodeType type, Token token) : m_type(type), m_token(token) {
}

Node::Node(NodeType type, NodePtr child) : m_type(type) {
  addChild(std::move(child));
}

void Node::addChild(NodePtr child) {
  m_children.push_back(std::move(child));
}

NodeType Node::type() const {
  return m_type;
}

const std::optional<Token>& Node::token() const {
  return m_token;
}

const std::vector<NodePtr>& Node::children() const {
  return m_children;
}

const Node* Node::child(NodeType type) const {
  for (const auto& node : m_children) {
    if (node->type() == type) {
      return node.get();
    }
  }
  return nullptr;
}
