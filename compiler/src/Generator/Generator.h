#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Gen {
class Generator {
 public:
  explicit Generator(Parser::NodePtr root);

  [[nodiscard]] std::string generate() const;

 private:
  [[nodiscard]] std::string genProgram(const Parser::Node* node) const;
  [[nodiscard]] std::string genScope(const Parser::Node* node) const;
  [[nodiscard]] std::string genStatement(const Parser::Node* node) const;

  [[nodiscard]] std::string genFunction(const Parser::Node* node) const;
  [[nodiscard]] std::string genReturn(const Parser::Node* node) const;

  [[nodiscard]] std::string genExpression(
      const Parser::Node* node, std::string_view resultReg) const;
  [[nodiscard]] std::string genBinaryOperation(
      const Parser::Node* node, std::string_view resultReg) const;
  [[nodiscard]] std::string genUnaryOperation(
      const Parser::Node* node, std::string_view resultReg) const;
  [[nodiscard]] std::string genBaseOperation(
      const Parser::Node* node, std::string_view resultReg) const;

  [[nodiscard]] static std::string instruction(
      std::string_view inst, const std::vector<std::string_view>& args);

  static std::string_view nextRegister(std::string_view reg);

  Parser::NodePtr m_root;
  Parser::Node* m_main = nullptr;
};
}  // namespace Cepheid::Gen