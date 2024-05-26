#include "Parser.h"

#include "Node/UnaryOperationNode.h"

#include <Parser/ParseException.h>
#include <Parser/Node/BinaryOperationNode.h>

#include <array>

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

NodePtr Parser::parseStatement() {
  if (NodePtr returnNode = parseReturnStatement()) {
    return returnNode;
  } else if (NodePtr functionDeclaration = parseFunctionDeclaration()) {
    return functionDeclaration;
  }
  return nullptr;
}

NodePtr Parser::parseReturnStatement() {
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

std::optional<Token> Parser::parseOperator(const std::vector<std::string_view>& operators) {
  auto addTokens = [](std::optional<Token> lhs, const std::optional<Token>& rhs) {
    if (!lhs) {
      return rhs;
    }
    *lhs->value += *rhs->value;
    return lhs;
  };

  std::optional<Token> result;
  for (const auto& op : operators) {
    std::optional<Token> match;
    for (int i = 0; i < op.size(); i++) {
      if (auto next = checkNextHasValue(TokenType::Operator, std::string(1, op[i]), i)) {
        match = addTokens(match, next);
      }
    }
    if (match && (!result || match->value->size() > result->value.value_or("").size())) {
      result = match;
    }
  }

  if(result) {
    consume(result->value->size()-1);
  }

  return result;
}

NodePtr Parser::parseExpression() {
  return parseEqualityOperation();
}

NodePtr Parser::parseEqualityOperation() {
  return parseComparisonOperation();
}

NodePtr Parser::parseComparisonOperation() {
  NodePtr operationNode = parseTermOperation();

  while (std::optional<Token> opToken = parseOperator({">", "<", ">=", "<="})) {
    // Convert operator token into an operation
    BinaryOperation operation = tokenToBinaryOperation(*opToken);

    // Create node for the operation
    auto binaryOpNode = std::make_unique<BinaryOperationNode>(operation);

    // Add the existing as the left hand side
    binaryOpNode->setLHS(std::move(operationNode));

    // Parse the right hand side
    NodePtr rhs = parseTermOperation();
    if (!rhs) {
      throw ParseException("Expected right hand expression");
    }
    binaryOpNode->setRHS(std::move(rhs));

    // Store as operationNode for the next step
    operationNode = std::move(binaryOpNode);
  }

  return operationNode;
}

NodePtr Parser::parseTermOperation() {
  NodePtr operationNode = parseFactorOperation();

  while (std::optional<Token> opToken = parseOperator({"+", "-"})) {
    // Convert operator token into an operation
    BinaryOperation operation = tokenToBinaryOperation(*opToken);

    // Create node for the operation
    auto binaryOpNode = std::make_unique<BinaryOperationNode>(operation);

    // Add the existing as the left hand side
    binaryOpNode->setLHS(std::move(operationNode));

    // Parse the right hand side
    NodePtr rhs = parseFactorOperation();
    if (!rhs) {
      throw ParseException("Expected right hand expression");
    }
    binaryOpNode->setRHS(std::move(rhs));

    // Store as operationNode for the next step
    operationNode = std::move(binaryOpNode);
  }

  return operationNode;
}

NodePtr Parser::parseFactorOperation() {
  NodePtr operationNode = parseUnaryOperation();

  while (std::optional<Token> opToken = parseOperator({"*", "/"})) {
    // Convert operator token into an operation
    BinaryOperation operation = tokenToBinaryOperation(*opToken);

    // Create node for the operation
    auto binaryOpNode = std::make_unique<BinaryOperationNode>(operation);

    // Add the existing as the left hand side
    binaryOpNode->setLHS(std::move(operationNode));

    // Parse the right hand side
    NodePtr rhs = parseUnaryOperation();
    if (!rhs) {
      throw ParseException("Expected right hand expression");
    }
    binaryOpNode->setRHS(std::move(rhs));

    // Store as operationNode for the next step
    operationNode = std::move(binaryOpNode);
  }

  return operationNode;
}

NodePtr Parser::parseUnaryOperation() {
  if (const std::optional<Token> opToken = parseOperator({"-", "!", "--", "++"})) {
    UnaryOperation operation = tokenToUnaryOperation(*opToken);

    auto unaryNode = std::make_unique<UnaryOperationNode>(operation);

    auto child = parseUnaryOperation();
    if (!child) {
      throw ParseException("Expected value in unary expression");
    }
    unaryNode->setOperand(std::move(child));
    return unaryNode;
  }
  return parseBaseOperation();
}

NodePtr Parser::parseBaseOperation() {
  if (checkNextHasValue(TokenType::IntegerLiteral)) {
    return Node::make(NodeType::IntegerLiteral, *consume());
  }
  if (checkNext(TokenType::OpenParen)) {
    consume();
    NodePtr node = parseExpression();
    if (!checkNext(TokenType::CloseParen)) {
      throw ParseException("Expected \")\" in expression");
    }
    consume();
    return node;
  }
  return {};
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

std::optional<Token> Parser::checkNextCompound(
    TokenType type, const std::vector<std::string>& sequence) {
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
