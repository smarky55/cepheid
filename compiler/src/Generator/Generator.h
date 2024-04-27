#pragma once

#include <Parser/ParseNode.h>

namespace Cepheid::Gen {
class Generator {
 public:
  explicit Generator(Parser::NodePtr root);

  [[nodiscard]] std::string generate();

 private:

  Parser::NodePtr m_root;
  Parser::Node* m_main = nullptr;
};
}  // namespace Cepheid::Gen