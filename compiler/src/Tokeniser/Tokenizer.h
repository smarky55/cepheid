#pragma once

#include <Tokeniser/Token.h>

#include <vector>
#include <string>

namespace Cepheid::Tokens {
class Tokeniser {
 public:
  explicit Tokeniser(std::string_view src);
  std::vector<Token> tokenise();

 private:
  std::optional<Token> readTokenOrKeyword();
  std::optional<Token> readTerminator();
  std::optional<Token> readOperator();
  std::optional<Token> readDelimiter();
  std::optional<Token> readBracket();
  std::optional<Token> readIntegerLiteral();

  std::optional<char> peek() const;
  std::optional<char> consume();

  size_t m_cursor = 0;
  SourceLocation m_currentLocation;
  std::string_view m_src;
};
}  // namespace Cepheid::Tokens