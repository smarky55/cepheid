#include "Generator.h"

#include <Generator/GenerationException.h>
#include <Parser/Node/BinaryOperationNode.h>
#include <Parser/Node/FunctionNode.h>
#include <Parser/Node/ScopeNode.h>
#include <Parser/Node/UnaryOperationNode.h>
#include <Tokeniser/Token.h>

#include <array>
#include <optional>
#include <sstream>

using namespace Cepheid::Gen;

using Cepheid::Parser::NodeType;

Generator::Generator(Parser::NodePtr root) : m_root(std::move(root)) {
}

std::string Generator::generate() const {
  return genProgram(m_root.get());
}

std::string Generator::genProgram(const Parser::Node* node) const {
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

std::string Generator::genScope(const Parser::ScopeNode* scope) const {
  std::string body;

  if (!scope) {
    throw GenerationException("Expected scope!");
  }

  for (const auto& child : scope->statements()) {
    body += genStatement(child.get());
  }
  return body;
}

std::string Generator::genStatement(const Parser::Node* node) const {
  switch (node->type()) {
    case NodeType::Function:
      return genFunction(node);
    case NodeType::ReturnStatement:
      return genReturn(node);
    default:
      break;
  }
  return {};
}

std::string Generator::genFunction(const Parser::Node* node) const {
  std::string body;

  const auto function = dynamic_cast<const Parser::FunctionNode*>(node);
  if (!function) {
    throw GenerationException("Expected function!");
  }

  // Label and prologue
  const std::string name = "cep_" + function->name();
  body += name + ":\n";
  body += instruction("push", {"rbp"});
  body += instruction("mov", {"rbp", "rsp"});
  body += instruction("sub", {"rsp", "32"});

  // Scope
  body += genScope(function->scope());

  return body;
}

std::string Generator::genReturn(const Parser::Node* node) const {
  std::string ret;
  // Compute expression
  if (!node->children().empty()) {
    ret += genExpression(node->children()[0].get(), "rbx");
    ret += instruction("mov", {"rax", "rbx"});
  }

  // Do the return
  ret += instruction("leave", {});
  ret += instruction("ret", {});
  return ret;
}

std::string Generator::genExpression(const Parser::Node* node, std::string_view resultReg) const {
  switch (node->type()) {
    case NodeType::BinaryOperation:
      return genBinaryOperation(node, resultReg);
    case NodeType::UnaryOperation:
      return genUnaryOperation(node, resultReg);
    default:
      return genBaseOperation(node, resultReg);
  }
}

std::string Generator::genBinaryOperation(const Parser::Node* node, std::string_view resultReg) const {
  const auto* binaryNode = dynamic_cast<const Parser::BinaryOperationNode*>(node);
  const std::string_view rhsReg = nextRegister(resultReg);
  std::string result = genExpression(binaryNode->lhs(), resultReg);
  result += genExpression(binaryNode->rhs(), rhsReg);

  switch (binaryNode->operation()) {
    case Parser::BinaryOperation::Add:
      result += instruction("add", {resultReg, rhsReg});
      break;
    case Parser::BinaryOperation::Subtract:
      result += instruction("sub", {resultReg, rhsReg});
      break;
    case Parser::BinaryOperation::Multiply:
      result += instruction("imul", {resultReg, rhsReg});
      break;
    case Parser::BinaryOperation::Divide:
      // TODO: IDIV needs its LHS in RDX:RAX first and stores the Quotient in RAX and remainder in RDX

      // result += instruction("idiv", {resultReg, rhsReg});
      // break;
    default:
      throw GenerationException("Unhandled binary operation");
  }

  return result;
}

std::string Generator::genUnaryOperation(const Parser::Node* node, std::string_view resultReg) const {
  const auto* unaryNode = dynamic_cast<const Parser::UnaryOperationNode*>(node);
  std::string result = genExpression(unaryNode->operand(), resultReg);

  switch (unaryNode->operation()) {
    case Parser::UnaryOperation::Negate:
      result += instruction("neg", {resultReg});
      break;
    case Parser::UnaryOperation::Not:
      result += instruction("not", {resultReg});
      break;
    case Parser::UnaryOperation::Decrement:
      result += instruction("dec", {resultReg});
      break;
    case Parser::UnaryOperation::Increment:
      result += instruction("inc", {resultReg});
      break;
    default:
      throw GenerationException("Unhandled unary operation");
  }

  return result;
}

std::string Generator::genBaseOperation(const Parser::Node* node, std::string_view resultReg) const {
  switch (node->type()) {
    case NodeType::IntegerLiteral:
      return instruction("mov", {resultReg, node->token()->value.value()});
    default:
      break;
  }
  return {};
}

std::string Generator::instruction(std::string_view inst, const std::vector<std::string_view>& args) {
  std::stringstream ss;
  ss << "  " << inst;
  if (!args.empty()) {
    ss << " " << args[0];
    for (size_t i = 1; i < args.size(); i++) {
      ss << ", " << args[i];
    }
  }
  ss << "\n";
  return ss.str();
}

std::string_view Generator::nextRegister(std::string_view reg) {
  static std::array<std::string_view, 12> registers{
      "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};
  auto current = std::ranges::find(registers, reg);
  if (current == registers.end() || current++ == registers.end()) {
    throw GenerationException("Unable to get next register");
  }
  return *current++;
}
