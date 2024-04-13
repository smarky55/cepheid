#include "Cepheid.h"

#include <Tokeniser/Tokenizer.h>

std::string Cepheid::compile(std::string_view src) const {

  std::vector<Token> tokens = Tokeniser(src).tokenise();

  return std::string();
}
