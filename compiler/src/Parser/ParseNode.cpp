#include "ParseNode.h"

using namespace Cepheid::Parser;

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
