#include "Tokenizer.h"

#include <Tokeniser/TokenisationException.h>

#include <array>
#include <format>
#include <map>

Tokeniser::Tokeniser(std::string_view src) : m_src(src) {
}

std::vector<Token> Tokeniser::tokenise() {
  std::vector<Token> tokens;
  while (peek()) {
    if (std::isspace(*peek())) {
      consume();
    } else if (std::optional<Token> token = readTokenOrKeyword()) {
      tokens.push_back(*token);
    } else if (std::optional<Token> token = readTerminator()) {
      tokens.push_back(*token);
    } else if (std::optional<Token> token = readBracket()) {
      tokens.push_back(*token);
    } else if (std::optional<Token> token = readOperator()) {
      tokens.push_back(*token);
    } else if (std::optional<Token> token = readDelimiter()) {
      tokens.push_back(*token);
    } else if (std::optional<Token> token = readIntegerLiteral()) {
      tokens.push_back(*token);
    } else {
      throw TokenisationException("Unexpected token.");
    }
  }
  return tokens;
}

static bool isIdentBeginChar(char character) {
  return std::isalpha(character) || character == '_';
}

static bool isIdentChar(char character) {
  return isIdentBeginChar(character) || std::isdigit(character);
}

static bool isKeyword(std::string_view identifier) {
  static std::array<std::string_view, 5> keywords{"func", "return", "import", "export", "module"};

  return std::ranges::find(keywords, identifier) != keywords.end();
}

std::optional<Token> Tokeniser::readTokenOrKeyword() {
  if (!isIdentBeginChar(*peek())) {
    return std::nullopt;
  }

  std::string val(1, *consume());

  for (std::optional<char> next = peek(); next && isIdentChar(*next); next = peek()) {
    val += *consume();
  }

  bool keyword = isKeyword(val);

  return std::optional<Token>(
      std::in_place, keyword ? TokenType::Keyword : TokenType::Identifier, val);
}

std::optional<Token> Tokeniser::readTerminator() {
  if (*peek() != ';') {
    return std::nullopt;
  }
  consume();
  return std::optional<Token>(std::in_place, TokenType::Terminator, std::nullopt);
}

std::optional<Token> Tokeniser::readOperator() {
  static const std::array operators{'+', '-', '/', '*', '.', '<', '>', '=', '!', '%'};

  if (std::ranges::find(operators, *peek()) == operators.end()) {
    return std::nullopt;
  }

  return std::optional<Token>(std::in_place, TokenType::Operator, std::string(1, *consume()));
}

std::optional<Token> Tokeniser::readDelimiter() {
  if (*peek() != ',') {
    return std::nullopt;
  }
  consume();
  return std::optional<Token>(std::in_place, TokenType::Delimiter, std::nullopt);
}

std::optional<Token> Tokeniser::readBracket() {
  static const std::map<char, TokenType> brackets{
      {'(', TokenType::OpenParen},
      {')', TokenType::CloseParen},
      {'{', TokenType::OpenBrace},
      {'}', TokenType::CloseBrace},
      {'[', TokenType::OpenBracket},
      {']', TokenType::CloseBracket}};

  if (auto it = brackets.find(*peek()); it != brackets.end()) {
    consume();
    return std::optional<Token>(std::in_place, it->second, std::nullopt);
  }

  return std::nullopt;
}

std::optional<Token> Tokeniser::readIntegerLiteral() {
  if (!std::isdigit(*peek())) {
    return std::nullopt;
  }

  std::string literal;
  while (std::isdigit(*peek())) {
    literal += *consume();
  }

  return std::optional<Token>(std::in_place, TokenType::IntegerLiteral, literal);
}

std::optional<char> Tokeniser::peek() const {
  if (m_cursor >= m_src.size()) {
    return std::nullopt;
  }

  return m_src.at(m_cursor);
}

std::optional<char> Tokeniser::consume() {
  if (m_cursor >= m_src.size()) {
    return std::nullopt;
  }
  return m_src.at(m_cursor++);
}
