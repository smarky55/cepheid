#include "Parser.h"

#include <Parser/ParseException.h>
#include <Parser/Node/BinaryOperation.h>
#include <Parser/Node/Conditional.h>
#include <Parser/Node/Function.h>
#include <Parser/Node/Loop.h>
#include <Parser/Node/Scope.h>
#include <Parser/Node/UnaryOperation.h>
#include <Parser/Node/VariableDeclaration.h>

using namespace Cepheid::Parser;

using Nodes::NodePtr;

using Cepheid::Tokens::Token;
using Cepheid::Tokens::TokenType;

Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens) {
}

NodePtr Parser::parse() {
  return parseModule();
}

NodePtr Parser::parseModule() {
  auto rootNode = std::make_unique<Nodes::Node>(Nodes::NodeType::Module);
  while (peek()) {
    if (auto node = parseStatement()) {
      rootNode->addChild(std::move(node));
    }
  }
  return rootNode;
}

NodePtr Parser::parseTypeName() {
  if (const auto name = checkNextHasValue(TokenType::Identifier)) {
    consume();
    auto typeName = std::make_unique<Nodes::Node>(Nodes::NodeType::TypeName);
    typeName->addChild(std::make_unique<Nodes::Node>(Nodes::NodeType::Identifier, *name));
    return typeName;
  }
  return nullptr;
}

NodePtr Parser::parseFunctionDeclaration() {
  if (!checkNextHasValue(TokenType::Keyword, "func")) {
    return nullptr;
  }
  consume();

  const std::optional<Token> name = checkNextHasValue(TokenType::Identifier);
  if (!name || !name->value) {
    throw ParseException("Expected function identifier");
  }
  consume();
  auto funcNode = std::make_unique<Nodes::Function>(name->value.value());

  {
    const std::optional<Token> openParen = checkNext(TokenType::OpenParen);
    if (!openParen) {
      throw ParseException("Expected \"(\" for function parameter list");
    }
    consume();

    // Loop parsing parameters while not close paren
    while (!checkNext(TokenType::CloseParen)) {
      // TODO: parseParameterDefinition
    }
    if (const auto closeParen = consume(); !closeParen || closeParen->type != TokenType::CloseParen) {
      throw ParseException("Expected \")\" after function parameter list");
    }
  }

  {
    const std::optional<Token> returnsOperator = checkNextCompound(TokenType::Operator, {"-", ">"});
    if (!returnsOperator) {
      throw ParseException("Expected \"->\" return type indicator");
    }
    consume(1);

    NodePtr returnType = parseTypeName();
    if (!returnType) {
      throw ParseException("Expected return typename");
    }
    funcNode->setReturnType(std::make_unique<Nodes::Node>(Nodes::NodeType::ReturnType, std::move(returnType)));
  }

  std::unique_ptr<Nodes::Scope> scope = parseScope();
  if (!scope) {
    throw ParseException("Expected function scope");
  }
  funcNode->setScope(std::move(scope));

  return funcNode;
}

std::unique_ptr<Nodes::Scope> Parser::parseScope() {
  if (!checkNext(TokenType::OpenBrace)) {
    return nullptr;
  }
  consume();

  auto scopeNode = std::make_unique<Nodes::Scope>();

  for (auto next = peek(); next && next->type != TokenType::CloseBrace; next = peek()) {
    if (NodePtr statementNode = parseStatement()) {
      scopeNode->addStatement(std::move(statementNode));
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
  } else if (NodePtr variableDeclaration = parseVariableDeclaration()) {
    return variableDeclaration;
  } else if (NodePtr ifStatement = parseIfStatement()) {
    return ifStatement;
  } else if (NodePtr loopStatement = parseLoopStatement()) {
    return loopStatement;
  }
  return nullptr;
}

NodePtr Parser::parseReturnStatement() {
  if (!checkNextHasValue(TokenType::Keyword, "return")) {
    return nullptr;
  }
  consume();

  auto returnNode = Nodes::Node::make(Nodes::NodeType::ReturnStatement);

  if (auto returnValue = parseExpression()) {
    returnNode->addChild(std::move(returnValue));
  }

  if (!checkNext(TokenType::Terminator)) {
    throw ParseException("Expected \";\"");
  }
  consume();

  return returnNode;
}

NodePtr Parser::parseVariableDeclaration() {
  // We want at least 2 identifiers, one for the type and one for the variable name
  if (!checkNextHasValue(TokenType::Identifier) || !checkNextHasValue(TokenType::Identifier, std::nullopt, 1)) {
    return nullptr;
  }

  NodePtr typeName = parseTypeName();
  const Token variableName = *consume();

  auto declaration = std::make_unique<Nodes::VariableDeclaration>(std::move(typeName), *variableName.value);
  if (checkNextHasValue(TokenType::Operator, "=")) {
    consume();
    NodePtr expressionNode = parseExpression();
    if (!expressionNode) {
      throw ParseException("Expected expression in variable declaration");
    }
    declaration->setExpression(std::move(expressionNode));
  }

  if (!checkNext(TokenType::Terminator)) {
    throw ParseException("Expected \";\"");
  }
  consume();

  return declaration;
}

NodePtr Parser::parseIfStatement() {
  if (!checkNextHasValue(TokenType::Keyword, "if")) {
    return nullptr;
  }
  consume();

  if (!checkNext(TokenType::OpenParen)) {
    throw ParseException("Expected \"(\" in if statement");
  }
  consume();

  NodePtr expression = parseExpression();
  if (!expression) {
    throw ParseException("Expected expression in if statement");
  }

  if (!checkNext(TokenType::CloseParen)) {
    throw ParseException("Expected \")\" in if statement");
  }
  consume();

  std::unique_ptr<Nodes::Scope> scope = parseScope();
  if (!scope) {
    throw ParseException("Expected scope for if statement");
  }

  return std::make_unique<Nodes::Conditional>(std::move(expression), std::move(scope));
}

NodePtr Parser::parseLoopStatement() {
  if (!(checkNextHasValue(TokenType::Keyword, "for") || checkNextHasValue(TokenType::Keyword, "while"))) {
    return nullptr;
  }
  const Token keywordToken = *consume();

  const bool isFor = keywordToken.value.value_or("while") == "for";

  if (!checkNext(TokenType::OpenParen)) {
    throw ParseException("Expected \"(\" in if statement");
  }
  consume();

  NodePtr initExpression;
  if (isFor) {
    initExpression = parseExpression();
    // This is allowed to be empty
    if (!checkNext(TokenType::Terminator)) {
      throw ParseException("Expected \";\" in for statement");
    }
    consume();
  }

  NodePtr conditionExpression = parseExpression();
  if (!conditionExpression) {
    throw ParseException("Expected condition");
  }

  NodePtr updateExpression;
  if (isFor) {
    if (!checkNext(TokenType::Terminator)) {
      throw ParseException("Expected \";\" in for statement");
    }
    consume();

    updateExpression = parseExpression();
    // This can be empty as well
  }

  if (!checkNext(TokenType::CloseParen)) {
    throw ParseException("Expected \")\" in loop statement");
  }
  consume();

  std::unique_ptr<Nodes::Scope> scope = parseScope();
  if (!scope) {
    throw ParseException("Expected scope for loop statement");
  }

  return std::make_unique<Nodes::Loop>(
      std::move(initExpression), std::move(conditionExpression), std::move(updateExpression), std::move(scope));
}

NodePtr Parser::parseExpressionStatement() {
  NodePtr expression = parseExpression();

  if (!checkNext(TokenType::Terminator)) {
    throw ParseException("Expected \";\"");
  }
  consume();
  return expression;
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
    for (size_t i = 0; i < op.size(); i++) {
      if (auto next = checkNextHasValue(TokenType::Operator, std::string(1, op[i]), i)) {
        match = addTokens(match, next);
      } else {
        match = std::nullopt;
        break;
      }
    }
    if (match && (!result || match->value->size() > result->value.value_or("").size())) {
      result = match;
    }
  }

  if (result) {
    consume(result->value->size() - 1);
  }

  return result;
}

NodePtr Parser::parseExpression() {
  return parseEqualityOperation();
}

NodePtr Parser::parserAssignmentOperation() {
  return parseBinaryOperation({"="}, &Parser::parseEqualityOperation);
}

NodePtr Parser::parseEqualityOperation() {
  return parseBinaryOperation({"==", "!="}, &Parser::parseComparisonOperation);
}

NodePtr Parser::parseComparisonOperation() {
  return parseBinaryOperation({">", "<", ">=", "<="}, &Parser::parseTermOperation);
}

NodePtr Parser::parseTermOperation() {
  return parseBinaryOperation({"+", "-"}, &Parser::parseFactorOperation);
}

NodePtr Parser::parseFactorOperation() {
  return parseBinaryOperation({"*", "/"}, &Parser::parseUnaryOperation);
}

NodePtr Parser::parseUnaryOperation() {
  if (const std::optional<Token> opToken = parseOperator({"-", "!", "--", "++"})) {
    Nodes::UnaryOperationType operation = Nodes::tokenToUnaryOperation(*opToken);

    auto unaryNode = std::make_unique<Nodes::UnaryOperation>(operation);

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
    return Nodes::Node::make(Nodes::NodeType::IntegerLiteral, *consume());
  }
  if (checkNextHasValue(TokenType::Identifier)) {
    return Nodes::Node::make(Nodes::NodeType::Identifier, *consume());
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

NodePtr Parser::parseBinaryOperation(
    const std::vector<std::string_view>& operators, BinaryOperationParser precedentFunc) {
  NodePtr operationNode = std::invoke(precedentFunc, this);

  while (std::optional<Token> opToken = parseOperator(operators)) {
    // Convert operator token into an operation
    Nodes::BinaryOperationType operation = Nodes::tokenToBinaryOperation(*opToken);

    // Create node for the operation
    auto binaryOpNode = std::make_unique<Nodes::BinaryOperation>(operation);

    // Add the existing as the left hand side
    binaryOpNode->setLHS(std::move(operationNode));

    // Parse the right hand side
    NodePtr rhs = std::invoke(precedentFunc, this);
    if (!rhs) {
      throw ParseException("Expected right hand expression");
    }
    binaryOpNode->setRHS(std::move(rhs));

    // Store as operationNode for the next step
    operationNode = std::move(binaryOpNode);
  }

  return operationNode;
}

std::optional<Token> Parser::checkNext(TokenType type, size_t offset) const {
  std::optional<Token> next = peek(offset);
  return next && next->type == type ? next : std::nullopt;
}

std::optional<Token> Parser::checkNextHasValue(
    TokenType type, const std::optional<std::string>& value, size_t offset) const {
  std::optional<Token> next = peek(offset);
  const bool isType = next && next->type == type;
  const bool containsAnyValue = next && next->value;
  const bool containsSpecificValue = containsAnyValue && value == next->value;
  const bool containsValue = value ? containsSpecificValue : containsAnyValue;
  return isType && containsValue ? next : std::nullopt;
}

std::optional<Token> Parser::checkNextCompound(TokenType type, const std::vector<std::string>& sequence) const {
  std::optional<Token> ret;

  for (size_t i = 0; i < sequence.size(); i++) {
    if (const auto token = checkNextHasValue(type, sequence[i], i)) {
      if (ret && ret->value) {
        ret->value.value() += sequence[i];
      } else {
        ret = token;
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
