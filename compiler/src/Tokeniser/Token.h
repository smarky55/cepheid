#pragma once

#include <optional>
#include <string>

namespace Cepheid::Tokens {
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

struct SourceLocation {
  size_t line = 0;
  size_t character = 0;
};

struct Token {
  TokenType type;
  std::optional<std::string> value;
  SourceLocation location;
};
}  // namespace Cepheid::Tokens