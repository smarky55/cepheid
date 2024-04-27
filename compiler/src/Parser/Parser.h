#pragma once

#include <Parser/ParseNode.h>
#include <Tokeniser/Token.h>

#include <memory>
#include <vector>

namespace Cepheid::Parser {
class Parser {
 public:
  explicit Parser(const std::vector<Tokens::Token>& tokens);

  [[nodiscard]] NodePtr parse();

 private:
  NodePtr parseProgram();

  NodePtr parseTypeName();

  NodePtr parseFunctionDeclaration();

  NodePtr parseScope();

  NodePtr parseStatement();

  NodePtr parseReturnStatement();

  NodePtr parseExpression();

  NodePtr parseEqualityOperation();

  NodePtr parseComparisonOperation();

  NodePtr parseTermOperation();

  NodePtr parseFactorOperation();

  NodePtr parseUnaryOperation();

  NodePtr parseBaseOperation();

  std::optional<Tokens::Token> checkNext(Tokens::TokenType type, size_t offset = 0);
  std::optional<Tokens::Token> checkNextHasValue(
      Tokens::TokenType type, std::optional<std::string> value = std::nullopt, size_t offset = 0);
  std::optional<Tokens::Token> checkNextCompound(
      Tokens::TokenType type, const std::vector<std::string>& sequence);
  std::optional<Tokens::Token> peek(size_t offset = 0) const;
  std::optional<Tokens::Token> consume(size_t offset = 0);

  size_t m_cursor = 0;
  std::vector<Tokens::Token> m_tokens;
};
}  // namespace Cepheid::Parser