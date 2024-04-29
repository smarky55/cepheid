#pragma once

#include <Parser/ParseNode.h>

namespace Cepheid::Gen {
class Generator {
 public:
  explicit Generator(Parser::NodePtr root);

  [[nodiscard]] std::string generate();

 private:
  std::string genProgram(const Parser::Node* node) const;
  std::string genScope(const Parser::Node* node) const;
  std::string genStatement(const Parser::Node* node) const;

  std::string genFunction(const Parser::Node* node) const;
  std::string genReturn(const Parser::Node* node) const;

  std::string genExpression(const Parser::Node* node, std::string_view reg) const;
  std::string genEqualityOperation(const Parser::Node* node, std::string_view reg) const;
  std::string genComparisonOperation(const Parser::Node* node, std::string_view reg) const;
  std::string genTermOperation(const Parser::Node* node, std::string_view reg) const;
  std::string genFactorOperation(const Parser::Node* node, std::string_view reg) const;
  std::string genUnaryOperation(const Parser::Node* node, std::string_view reg) const;
  std::string genBaseOperation(const Parser::Node* node, std::string_view reg) const;

  std::string instruction(std::string_view inst, const std::vector<std::string_view>& args) const;

  Parser::NodePtr m_root;
  Parser::Node* m_main = nullptr;
};
}  // namespace Cepheid::Gen