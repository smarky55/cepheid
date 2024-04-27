#include "Parser.h"

#include <Parser/ParseException.h>

using namespace Cepheid::Parser;

using Cepheid::Tokens::Token;
using Cepheid::Tokens::TokenType;

Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens) {
}

NodePtr Parser::parse() {
  return parseProgram();
}

NodePtr Parser::parseProgram() {
  auto rootNode = std::make_unique<Node>(NodeType::Program);
  while (peek()) {
    if (auto node = parseStatement()) {
      rootNode->addChild(std::move(node));
    }
  }
  return rootNode;
}

NodePtr Parser::parseTypeName() {
  if (auto name = checkNextHasValue(TokenType::Identifier)) {
    consume();
    auto typeName = std::make_unique<Node>(NodeType::TypeName);
    typeName->addChild(std::make_unique<Node>(NodeType::Identifier, *name));
    return typeName;
  }
  return nullptr;
}

NodePtr Parser::parseFunctionDeclaration() {
  if (!checkNextHasValue(TokenType::Keyword, "func")) {
    return nullptr;
  }
  consume();

  auto funcNode = std::make_unique<Node>(NodeType::Function);

  {
    NodePtr functionName = parseTypeName();
    if (!functionName) {
      throw ParseException("Expected function identifier");
    }
    funcNode->addChild(std::move(functionName));
  }

  {
    std::optional<Token> openParen = checkNext(TokenType::OpenParen);
    if (!openParen) {
      throw ParseException("Expected \"(\" for function parameter list");
    }
    consume();

    // Loop parsing parameters while not close paren
    while (!checkNext(TokenType::CloseParen)) {
      // parseParameterDefinition
    }
    if (auto closeParen = consume(); !closeParen || closeParen->type != TokenType::CloseParen) {
      throw ParseException("Expected \")\" after function parameter list");
    }
  }

  {
    std::optional<Token> returnsOperator = checkNextCompound(TokenType::Operator, {"-", ">"});
    if (!returnsOperator) {
      throw ParseException("Expected \"->\" return type indicator");
    }
    consume(1);

    NodePtr returnType = parseTypeName();
    if (!returnType) {
      throw ParseException("Expected return typename");
    }
    funcNode->addChild(std::make_unique<Node>(NodeType::ReturnType, std::move(returnType)));
  }

  NodePtr scope = parseScope();
  if (!scope) {
    throw ParseException("Expected function scope");
  }
  funcNode->addChild(std::move(scope));

  return funcNode;
}

NodePtr Parser::parseScope() {
  if (!checkNext(TokenType::OpenBrace)) {
    return nullptr;
  }
  consume();

  auto scopeNode = std::make_unique<Node>(NodeType::Scope);

  for (auto next = peek(); next && next->type != TokenType::CloseBrace; next = peek()) {
    if (NodePtr statementNode = parseStatement()) {
      scopeNode->addChild(std::move(statementNode));
    }
  }

  if (!checkNext(TokenType::CloseBrace)) {
    throw ParseException("Expected \"}\"");
  }
  consume();

  return scopeNode;
}

NodePtr Cepheid::Parser::Parser::parseStatement() {
  if (NodePtr returnNode = parseReturnStatement()) {
    return returnNode;
  } else if (NodePtr functionDeclaration = parseFunctionDeclaration()) {
    return functionDeclaration;
  }
  return nullptr;
}

NodePtr Cepheid::Parser::Parser::parseReturnStatement() {
  if (!checkNextHasValue(TokenType::Keyword, "return")) {
    return nullptr;
  }
  consume();

  auto returnNode = Node::make(NodeType::ReturnStatement);

  if (auto returnValue = parseExpression()) {
    returnNode->addChild(std::move(returnValue));
  }

  if (!checkNext(TokenType::Terminator)) {
    throw ParseException("Expected \";\"");
  }
  consume();

  return returnNode;
}

NodePtr Cepheid::Parser::Parser::parseExpression() {
  return parseEqualityOperation();
}

NodePtr Cepheid::Parser::Parser::parseEqualityOperation() {
  return parseComparisonOperation();
}

NodePtr Cepheid::Parser::Parser::parseComparisonOperation() {
  NodePtr binaryNode = parseTermOperation();

  while (checkNextHasValue(TokenType::Operator, ">") ||
         checkNextHasValue(TokenType::Operator, "<")) {
    // Make a new node and add the existing as the left hand side
    binaryNode = Node::make(NodeType::BinaryOperation, std::move(binaryNode));

    // Add a node for the operator
    binaryNode->addChild(Node::make(NodeType::Operator, *consume()));

    // Parse the right hand side
    NodePtr rhs = parseTermOperation();
    if (!rhs) {
      throw ParseException("Expected right hand expression");
    }
    binaryNode->addChild(std::move(rhs));
  }

  return binaryNode;
}

NodePtr Cepheid::Parser::Parser::parseTermOperation() {
  NodePtr binaryNode = parseFactorOperation();

  while (checkNextHasValue(TokenType::Operator, "+") ||
         checkNextHasValue(TokenType::Operator, "-")) {
    // Make a new node and add the existing as the left hand side
    binaryNode = Node::make(NodeType::BinaryOperation, std::move(binaryNode));

    // Add a node for the operator
    binaryNode->addChild(Node::make(NodeType::Operator, *consume()));

    // Parse the right hand side
    NodePtr rhs = parseUnaryOperation();
    if (!rhs) {
      throw ParseException("Expected right hand expression");
    }
    binaryNode->addChild(std::move(rhs));
  }

  return binaryNode;
}

NodePtr Cepheid::Parser::Parser::parseFactorOperation() {
  NodePtr binaryNode = parseUnaryOperation();

  while (checkNextHasValue(TokenType::Operator, "/") ||
         checkNextHasValue(TokenType::Operator, "*")) {
    // Make a new node and add the existing as the left hand side
    binaryNode = Node::make(NodeType::BinaryOperation, std::move(binaryNode));

    // Add a node for the operator
    binaryNode->addChild(Node::make(NodeType::Operator, *consume()));

    // Parse the right hand side
    NodePtr rhs = parseUnaryOperation();
    if (!rhs) {
      throw ParseException("Expected right hand expression");
    }
    binaryNode->addChild(std::move(rhs));
  }

  return binaryNode;
}

NodePtr Cepheid::Parser::Parser::parseUnaryOperation() {
  if (checkNextHasValue(TokenType::Operator, "-") || checkNextHasValue(TokenType::Operator, "!")) {
    auto unaryNode = Node::make(NodeType::UnaryOperation);
    unaryNode->addChild(Node::make(NodeType::Operator, *consume()));
    auto child = parseUnaryOperation();
    if (!child) {
      throw ParseException("Expected value in unary expression");
    }
    unaryNode->addChild(std::move(child));
    return unaryNode;
  }
  return parseBaseOperation();
}

NodePtr Cepheid::Parser::Parser::parseBaseOperation() {
  if (checkNextHasValue(TokenType::IntegerLiteral)) {
    return Node::make(NodeType::IntegerLiteral, *consume());
  }
  return NodePtr();
}

std::optional<Token> Parser::checkNext(TokenType type, size_t offset) {
  std::optional<Token> next = peek(offset);
  return next && next->type == type ? next : std::nullopt;
}

std::optional<Token> Parser::checkNextHasValue(
    TokenType type, std::optional<std::string> value, size_t offset) {
  std::optional<Token> next = peek(offset);
  bool isType = next && next->type == type;
  bool containsAnyValue = next && next->value;
  bool containsSpecificValue = containsAnyValue && value == next->value;
  bool containsValue = value ? containsSpecificValue : containsAnyValue;
  return isType && containsValue ? next : std::nullopt;
}

std::optional<Token> Cepheid::Parser::Parser::checkNextCompound(
    Tokens::TokenType type, const std::vector<std::string>& sequence) {
  std::optional<Token> ret;

  for (size_t i = 0; i < sequence.size(); i++) {
    if (auto token = checkNextHasValue(type, sequence[i], i)) {
      if (!ret) {
        ret = token;
      } else {
        ret->value.value() += sequence[i];
      }
    } else {
      return std::nullopt;
    }
  }

  return ret;
}

std::optional<Token> Parser::peek(size_t offset) const {
  if (m_cursor + offset >= m_tokens.size()) {
    return std::nullopt;
  }
  return m_tokens.at(m_cursor + offset);
}

std::optional<Token> Parser::consume(size_t offset) {
  if (m_cursor + offset >= m_tokens.size()) {
    return std::nullopt;
  }
  std::optional<Token> ret{std::in_place, m_tokens.at(m_cursor)};
  m_cursor += 1 + offset;
  return ret;
}