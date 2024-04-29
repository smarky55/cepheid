#include "Generator.h"

#include <Tokeniser/Token.h>

#include <optional>
#include <sstream>

using namespace Cepheid::Gen;

using Cepheid::Parser::NodeType;

Cepheid::Gen::Generator::Generator(Parser::NodePtr root) : m_root(std::move(root)) {
}

std::string Cepheid::Gen::Generator::generate() {
  return genProgram(m_root.get());
}

std::string Cepheid::Gen::Generator::genProgram(const Parser::Node* node) const {
  std::string program = R"(bits 64
default rel

segment .text
global _entry
extern _CRT_INIT
extern ExitProcess

_entry:
  push rbp
  mov rbp, rsp
  sub rsp, 32

  call _CRT_INIT

  call cep_main
  
  mov rcx, rax
  call ExitProcess

)";

  for (const auto& child : node->children()) {
    program += genStatement(child.get());
  }

  return program;
}

std::string Cepheid::Gen::Generator::genScope(const Parser::Node* node) const {
  std::string body;
  for (const auto& child : node->children()) {
    body += genStatement(child.get());
  }
  return body;
}

std::string Cepheid::Gen::Generator::genStatement(const Parser::Node* node) const {
  switch (node->type()) {
    case NodeType::Function:
      return genFunction(node);
    case NodeType::ReturnStatement:
      return genReturn(node);
    default:
      break;
  }
  return std::string();
}

std::string Cepheid::Gen::Generator::genFunction(const Parser::Node* node) const {
  std::string body;

  // Label and prologue
  const Parser::Node* typeName = node->child(NodeType::TypeName);
  const Parser::Node* ident = typeName->child(NodeType::Identifier);
  std::string name = "cep_" + ident->token()->value.value();
  body += name + ":\n";
  body += instruction("push", {"rbp"});
  body += instruction("mov", {"rbp", "rsp"});
  body += instruction("sub", {"rsp", "32"});

  // Scope
  body += genScope(node->child(NodeType::Scope));

  return body;
}

std::string Cepheid::Gen::Generator::genReturn(const Parser::Node* node) const {
  std::string ret;
  // Compute expression
  if (node->children().size()) {
    ret += genExpression(node->children()[0].get(), "rax");
  }

  // Do the return
  ret += instruction("leave", {});
  ret += instruction("ret", {});
  return ret;
}

std::string Cepheid::Gen::Generator::genExpression(
    const Parser::Node* node, std::string_view reg) const {
  return genEqualityOperation(node, reg);
}

std::string Cepheid::Gen::Generator::genEqualityOperation(
    const Parser::Node* node, std::string_view reg) const {
  return genComparisonOperation(node, reg);
}

std::string Cepheid::Gen::Generator::genComparisonOperation(
    const Parser::Node* node, std::string_view reg) const {
  return genTermOperation(node, reg);
}

std::string Cepheid::Gen::Generator::genTermOperation(
    const Parser::Node* node, std::string_view reg) const {
  return genFactorOperation(node, reg);
}

std::string Cepheid::Gen::Generator::genFactorOperation(
    const Parser::Node* node, std::string_view reg) const {
  return genUnaryOperation(node, reg);
}

std::string Cepheid::Gen::Generator::genUnaryOperation(
    const Parser::Node* node, std::string_view reg) const {
  return genBaseOperation(node, reg);
}

std::string Cepheid::Gen::Generator::genBaseOperation(
    const Parser::Node* node, std::string_view reg) const {
  switch (node->type()) {
    case NodeType::IntegerLiteral:
      return instruction("mov", {reg, node->token()->value.value()});
    default:
      break;
  }
  return std::string();
}

std::string Cepheid::Gen::Generator::instruction(
    std::string_view inst, const std::vector<std::string_view>& args) const {
  std::stringstream ss;
  ss << "  " << inst;
  if (args.size()) {
    ss << " " << args[0];
    for (size_t i = 1; i < args.size(); i++) {
      ss << ", " << args[i];
    }
  }
  ss << "\n";
  return ss.str();
}
