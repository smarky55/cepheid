#pragma once

#include <optional>
#include <string>

enum class TokenType {
  Identifier,
  Keyword,
  Terminator,
  OpenParen,
  CloseParen,
  OpenBracket,
  CloseBracket,
  OpenBrace,
  CloseBrace,
  Operator,
  Delimiter,
  IntegerLiteral
};

struct Token {
  TokenType type;
  std::optional<std::string> value;
};