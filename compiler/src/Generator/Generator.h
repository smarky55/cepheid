#pragma once

#include <Parser/Node/ParseNode.h>

namespace Cepheid::Parser::Nodes {
class Scope;
}

namespace Cepheid::Gen {
class Context;
class Location;
class Register;

class Generator {
 public:
  explicit Generator(Parser::Nodes::NodePtr root);

  [[nodiscard]] std::string generate() const;

 private:
  [[nodiscard]] std::string genProgram(const Parser::Nodes::Node* node, Context& context) const;
  [[nodiscard]] std::string genScope(const Parser::Nodes::Scope* scope, size_t stackOffset, Context& context) const;
  [[nodiscard]] std::string genStatement(const Parser::Nodes::Node* node, Context& context) const;

  [[nodiscard]] std::string genFunction(const Parser::Nodes::Node* node, Context& context) const;
  [[nodiscard]] std::string genReturn(const Parser::Nodes::Node* node, Context& context) const;
  [[nodiscard]] std::string genVariableDeclaration(const Parser::Nodes::Node* node, Context& context) const;

  [[nodiscard]] std::string genExpression(const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) const;
  [[nodiscard]] std::string genBinaryOperation(const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) const;
  [[nodiscard]] std::string genUnaryOperation(const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) const;
  [[nodiscard]] std::string genBaseOperation(const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) const;

  [[nodiscard]] static std::string instruction(std::string_view inst, const std::vector<std::string_view>& args);

  static const Register& nextRegister(const Location& location);

  Parser::Nodes::NodePtr m_root;
  Parser::Nodes::Node* m_main = nullptr;
};
}  // namespace Cepheid::Gen