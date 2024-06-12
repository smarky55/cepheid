#pragma once

#include <Parser/Node/ParseNode.h>

#include <sstream>

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

  [[nodiscard]] std::string generate();

 private:
  void genProgram(const Parser::Nodes::Node* node, Context& context);
  void genScope(const Parser::Nodes::Scope* scope, Context& context);
  void genStatement(const Parser::Nodes::Node* node, Context& context);

  void genFunction(const Parser::Nodes::Node* node, Context& context);
  void genReturn(const Parser::Nodes::Node* node, Context& context);
  void genVariableDeclaration(const Parser::Nodes::Node* node, Context& context);
  void genConditional(const Parser::Nodes::Node* node, Context& context);

  void genExpression(
      const Parser::Nodes::Node* node,
      const Location& resultLocation,
      Context& context);
  void genBinaryOperation(
      const Parser::Nodes::Node* node,
      const Location& resultLocation,
      Context& context);
  void genUnaryOperation(
      const Parser::Nodes::Node* node,
      const Location& resultLocation,
      Context& context);
  void genBaseOperation(
      const Parser::Nodes::Node* node,
      const Location& resultLocation,
      Context& context);

  void writeInstruction(std::string_view inst, const std::vector<std::string_view>& args);
  void writeLabel(std::string_view label);

  static const Register& nextRegister(const Location& location);

  Parser::Nodes::NodePtr m_root;
  Parser::Nodes::Node* m_main = nullptr;
  std::stringstream m_program;
};
}  // namespace Cepheid::Gen