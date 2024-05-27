#pragma once

#include <Parser/Node/ParseNode.h>
#include <Tokeniser/Token.h>

#include <memory>
#include <vector>

namespace Cepheid::Parser {
namespace Nodes {
class Scope;
class VariableDeclaration;
}  // namespace Nodes

class Parser {
 public:
  explicit Parser(const std::vector<Tokens::Token>& tokens);

  [[nodiscard]] Nodes::NodePtr parse();

 private:
  Nodes::NodePtr parseProgram();

  Nodes::NodePtr parseTypeName();

  Nodes::NodePtr parseFunctionDeclaration();

  std::unique_ptr<Nodes::Scope> parseScope();

  Nodes::NodePtr parseStatement();

  Nodes::NodePtr parseReturnStatement();

  Nodes::NodePtr parseVariableDeclaration();

  std::optional<Tokens::Token> parseOperator(const std::vector<std::string_view>& operators);

  Nodes::NodePtr parseExpression();

  Nodes::NodePtr parserAssignmentOperation();

  Nodes::NodePtr parseEqualityOperation();

  Nodes::NodePtr parseComparisonOperation();

  Nodes::NodePtr parseTermOperation();

  Nodes::NodePtr parseFactorOperation();

  Nodes::NodePtr parseUnaryOperation();

  Nodes::NodePtr parseBaseOperation();

  using BinaryOperationParser = Nodes::NodePtr (Parser::*)();

  Nodes::NodePtr parseBinaryOperation(
      const std::vector<std::string_view>& operators, BinaryOperationParser precedentFunc);

  [[nodiscard]] std::optional<Tokens::Token> checkNext(Tokens::TokenType type, size_t offset = 0) const;
  [[nodiscard]] std::optional<Tokens::Token> checkNextHasValue(
      Tokens::TokenType type, const std::optional<std::string>& value = std::nullopt, size_t offset = 0) const;
  [[nodiscard]] std::optional<Tokens::Token> checkNextCompound(
      Tokens::TokenType type, const std::vector<std::string>& sequence) const;
  [[nodiscard]] std::optional<Tokens::Token> peek(size_t offset = 0) const;
  std::optional<Tokens::Token> consume(size_t offset = 0);

  size_t m_cursor = 0;
  std::vector<Tokens::Token> m_tokens;
};
}  // namespace Cepheid::Parser
