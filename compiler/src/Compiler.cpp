#include "Compiler.h"

#include <Generator/Generator.h>
#include <Parser/Parser.h>
#include <Tokeniser/Tokenizer.h>

using namespace Cepheid;

std::string Compiler::compile(std::string_view src) const {
  std::vector<Tokens::Token> tokens = Tokens::Tokeniser(src).tokenise();
  std::unique_ptr<Parser::Nodes::Node> parseTree = Parser::Parser(tokens).parse();
  return Gen::Generator(std::move(parseTree)).generate();
}
