#include "Compiler.h"

#include <Tokeniser/Tokenizer.h>
#include <Parser/Parser.h>

using namespace Cepheid;

std::string Compiler::compile(std::string_view src) const {
  std::vector<Tokens::Token> tokens = Tokens::Tokeniser(src).tokenise();
  std::unique_ptr<Parser::Node> parseTree = Parser::Parser(tokens).parse();
  return std::string();
}
