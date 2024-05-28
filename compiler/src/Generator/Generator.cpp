#include "Generator.h"

#include <Generator/Context.h>
#include <Generator/GenerationException.h>
#include <Parser/Node/BinaryOperation.h>
#include <Parser/Node/Function.h>
#include <Parser/Node/Scope.h>
#include <Parser/Node/UnaryOperation.h>
#include <Tokeniser/Token.h>

#include <array>
#include <optional>
#include <sstream>
#include <Parser/Node/VariableDeclaration.h>

using namespace Cepheid::Gen;

using Cepheid::Parser::Nodes::NodeType;

Generator::Generator(Parser::Nodes::NodePtr root) : m_root(std::move(root)) {
}

std::string Generator::generate() const {
  Context context;
  return genProgram(m_root.get(), context);
}

std::string Generator::genProgram(const Parser::Nodes::Node* node, Context& context) const {
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
    program += genStatement(child.get(), context);
  }

  return program;
}

std::string Generator::genScope(const Parser::Nodes::Scope* scope, size_t stackOffset, Context& context) const {
  context.push();

  std::string body;

  if (!scope) {
    throw GenerationException("Expected scope!");
  }

  for (const auto& child : scope->statements()) {
    body += genStatement(child.get(), context);
  }

  context.pop();
  return body;
}

std::string Generator::genStatement(const Parser::Nodes::Node* node, Context& context) const {
  switch (node->type()) {
    case NodeType::Function:
      return genFunction(node, context);
    case NodeType::ReturnStatement:
      return genReturn(node, context);
    case NodeType::VariableDeclaration:
      return genVariableDeclaration(node, context);
    default:
      break;
  }
  return {};
}

std::string Generator::genFunction(const Parser::Nodes::Node* node, Context& context) const {
  std::string body;

  const auto function = dynamic_cast<const Parser::Nodes::Function*>(node);
  if (!function) {
    throw GenerationException("Expected function!");
  }

  const size_t stackSpace = 32 + ((function->requiredStackSpace() + 15) / 16) * 16;
  context.push();
  // TODO update context for function parameters

  // Label and prologue
  const std::string name = "cep_" + function->name();
  body += name + ":\n";
  body += instruction("push", {"rbp"});
  body += instruction("mov", {"rbp", "rsp"});

  body += instruction("sub", {"rsp", std::to_string(stackSpace)});

  // Scope
  body += genScope(function->scope(), 0, context);

  context.pop();
  return body;
}

std::string Generator::genReturn(const Parser::Nodes::Node* node, Context& context) const {
  std::string ret;
  // Compute expression
  if (!node->children().empty()) {
    ret += genExpression(node->children()[0].get(), "rbx", context);
    ret += instruction("mov", {"rax", "rbx"});
  }

  // Do the return
  ret += instruction("leave", {});
  ret += instruction("ret", {});
  return ret;
}

std::string Generator::genVariableDeclaration(const Parser::Nodes::Node* node, Context& context) const {
  const auto variableDeclaration = dynamic_cast<const Parser::Nodes::VariableDeclaration*>(node);
  if (!variableDeclaration) {
    throw GenerationException("Expected variable declaration");
  }

  context.addVariable(variableDeclaration);

  const std::optional<Context::VariableContext> varContext = context.variable(variableDeclaration->name());
  if (!varContext) {
    throw GenerationException("Somehow failed to add variable to context");
  }

  std::string ret;
  if (variableDeclaration->expression()) {
    ret = genExpression(variableDeclaration->expression(), "rbx", context);
    ret += instruction("mov", {"[ rsp - " + std::to_string(varContext->m_offset + varContext->m_size) + " ]", "rbx"});
  }

  return ret;
}

std::string Generator::genExpression(
    const Parser::Nodes::Node* node, std::string_view resultReg, Context& context) const {
  switch (node->type()) {
    case NodeType::BinaryOperation:
      return genBinaryOperation(node, resultReg, context);
    case NodeType::UnaryOperation:
      return genUnaryOperation(node, resultReg, context);
    default:
      return genBaseOperation(node, resultReg, context);
  }
}

std::string Generator::genBinaryOperation(
    const Parser::Nodes::Node* node, std::string_view resultReg, Context& context) const {
  const auto* binaryNode = dynamic_cast<const Parser::Nodes::BinaryOperation*>(node);
  const std::string_view rhsReg = nextRegister(resultReg);
  std::string result = genExpression(binaryNode->lhs(), resultReg, context);
  result += genExpression(binaryNode->rhs(), rhsReg, context);

  switch (binaryNode->operation()) {
    case Parser::Nodes::BinaryOperationType::Add:
      result += instruction("add", {resultReg, rhsReg});
      break;
    case Parser::Nodes::BinaryOperationType::Subtract:
      result += instruction("sub", {resultReg, rhsReg});
      break;
    case Parser::Nodes::BinaryOperationType::Multiply:
      result += instruction("imul", {resultReg, rhsReg});
      break;
    case Parser::Nodes::BinaryOperationType::Divide:
      // TODO: IDIV needs its LHS in RDX:RAX first and stores the Quotient in RAX and remainder in RDX

      // result += instruction("idiv", {resultReg, rhsReg});
      // break;
    default:
      throw GenerationException("Unhandled binary operation");
  }

  return result;
}

std::string Generator::genUnaryOperation(
    const Parser::Nodes::Node* node, std::string_view resultReg, Context& context) const {
  const auto* unaryNode = dynamic_cast<const Parser::Nodes::UnaryOperation*>(node);
  std::string result = genExpression(unaryNode->operand(), resultReg, context);

  switch (unaryNode->operation()) {
    case Parser::Nodes::UnaryOperationType::Negate:
      result += instruction("neg", {resultReg});
      break;
    case Parser::Nodes::UnaryOperationType::Not:
      result += instruction("not", {resultReg});
      break;
    case Parser::Nodes::UnaryOperationType::Decrement:
      result += instruction("dec", {resultReg});
      break;
    case Parser::Nodes::UnaryOperationType::Increment:
      result += instruction("inc", {resultReg});
      break;
    default:
      throw GenerationException("Unhandled unary operation");
  }

  return result;
}

std::string Generator::genBaseOperation(
    const Parser::Nodes::Node* node, std::string_view resultReg, Context& context) const {
  switch (node->type()) {
    case NodeType::IntegerLiteral:
      return instruction("mov", {resultReg, node->token()->value.value()});
    case NodeType::Identifier: {
      const std::string identName = node->token()->value.value();
      const std::optional<Context::VariableContext> varContext = context.variable(identName);

      if (!varContext) {
        throw GenerationException("Unknown identifier in expression");
      }

      return instruction(
          "mov", {resultReg, "[ rsp - " + std::to_string(varContext->m_offset + varContext->m_size) + " ]"});
    }
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
