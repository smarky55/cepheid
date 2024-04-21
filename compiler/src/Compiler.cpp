#include "Compiler.h"

#include <Tokeniser/Tokenizer.h>

using namespace Cepheid;

std::string Compiler::compile(std::string_view src) const {
  std::vector<Tokens::Token> tokens = Tokens::Tokeniser(src).tokenise();
  return std::string();
}
