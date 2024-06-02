#include "Generator.h"

#include <Generator/Context.h>
#include <Generator/GenerationException.h>
#include <Generator/Location.h>
#include <Generator/MemoryLocation.h>
#include <Generator/Register.h>
#include <Parser/Node/BinaryOperation.h>
#include <Parser/Node/Conditional.h>
#include <Parser/Node/Function.h>
#include <Parser/Node/Scope.h>
#include <Parser/Node/UnaryOperation.h>
#include <Parser/Node/VariableDeclaration.h>
#include <Tokeniser/Token.h>

#include <array>
#include <optional>
#include <sstream>

using namespace Cepheid::Gen;

using Cepheid::Parser::Nodes::NodeType;

namespace {
std::array REGISTERS{
    Register{Register::Kind::Original, "a"},
    Register{Register::Kind::Original, "b"},
    Register{Register::Kind::Original, "c"},
    Register{Register::Kind::Original, "d"},
    Register{Register::Kind::AMD64, "r8"},
    Register{Register::Kind::AMD64, "r9"},
    Register{Register::Kind::AMD64, "r10"},
    Register{Register::Kind::AMD64, "r11"},
    Register{Register::Kind::AMD64, "r12"},
    Register{Register::Kind::AMD64, "r13"},
    Register{Register::Kind::AMD64, "r14"},
    Register{Register::Kind::AMD64, "r15"}};
}

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

std::string Generator::genScope(const Parser::Nodes::Scope* scope, Context& context) const {
  context.pushScope();

  std::string body;

  if (!scope) {
    throw GenerationException("Expected scope!");
  }

  for (const auto& child : scope->statements()) {
    body += genStatement(child.get(), context);
  }

  context.popScope();
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
    case NodeType::Conditional:
      return genConditional(node, context);
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
  context.pushFunction();
  // TODO update context for function parameters

  // Label and prologue
  const std::string name = "cep_" + function->name();
  body += name + ":\n";
  body += instruction("push", {"rbp"});
  body += instruction("mov", {"rbp", "rsp"});

  body += instruction("sub", {"rsp", std::to_string(stackSpace)});

  // Scope
  body += genScope(function->scope(), context);

  context.popFunction();
  return body;
}

std::string Generator::genReturn(const Parser::Nodes::Node* node, Context& context) const {
  std::string ret;
  // Compute expression
  if (!node->children().empty()) {
    ret += genExpression(node->children()[0].get(), REGISTERS[1], context);
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
    const MemoryLocation location{"[ rsp + " + std::to_string(varContext->offset) + " ]"};
    const Register& reg = nextRegister(location);
    ret = genExpression(variableDeclaration->expression(), reg, context);
    ret += instruction("mov", {location.asAsm(varContext->size), reg.asAsm(varContext->size)});
  }

  return ret;
}

std::string Generator::genConditional(const Parser::Nodes::Node* node, Context& context) const {
  const auto conditional = dynamic_cast<const Parser::Nodes::Conditional*>(node);
  if (!conditional) {
    throw GenerationException("Expected conditional");
  }
  const size_t labelIndex = context.nextLocalLabel();
  const std::string label = ".L" + std::to_string(labelIndex);
  const Register& conditionRegister = REGISTERS[0];
  std::string ret = genExpression(conditional->expression(), conditionRegister, context);
  ret += instruction("cmp", {conditionRegister.asAsm(1), "0"});
  ret += instruction("je", {label});
  ret += genScope(conditional->scope(), context);
  ret += label + ":\n";
  return ret;
}

std::string Generator::genExpression(
    const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) const {
  switch (node->type()) {
    case NodeType::BinaryOperation:
      return genBinaryOperation(node, resultLocation, context);
    case NodeType::UnaryOperation:
      return genUnaryOperation(node, resultLocation, context);
    default:
      return genBaseOperation(node, resultLocation, context);
  }
}

std::string Generator::genBinaryOperation(
    const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) const {
  const auto* binaryNode = dynamic_cast<const Parser::Nodes::BinaryOperation*>(node);
  const Register& rhsReg = nextRegister(resultLocation);
  std::string result = genExpression(binaryNode->lhs(), resultLocation, context);
  result += genExpression(binaryNode->rhs(), rhsReg, context);

  switch (binaryNode->operation()) {
    case Parser::Nodes::BinaryOperationType::Add:
      result += instruction("add", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      break;
    case Parser::Nodes::BinaryOperationType::Subtract:
      result += instruction("sub", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      break;
    case Parser::Nodes::BinaryOperationType::Multiply:
      result += instruction("imul", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      break;
    case Parser::Nodes::BinaryOperationType::Divide:
      // TODO: IDIV needs its LHS in RDX:RAX first and stores the Quotient in RAX and remainder in RDX
      // result += instruction("idiv", {resultReg, rhsReg});
      break;
    case Parser::Nodes::BinaryOperationType::Equal:
      result += instruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      result += instruction("mov", {resultLocation.asAsm(8), "0"});
      result += instruction("sete", {resultLocation.asAsm(1)});
      break;
    case Parser::Nodes::BinaryOperationType::NotEqual:
      result += instruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      result += instruction("mov", {resultLocation.asAsm(8), "0"});
      result += instruction("setne", {resultLocation.asAsm(1)});
      break;
    case Parser::Nodes::BinaryOperationType::GreaterEqual:
      result += instruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      result += instruction("mov", {resultLocation.asAsm(8), "0"});
      result += instruction("setge", {resultLocation.asAsm(1)});
      break;
    case Parser::Nodes::BinaryOperationType::GreaterThan:
      result += instruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      result += instruction("mov", {resultLocation.asAsm(8), "0"});
      result += instruction("setg", {resultLocation.asAsm(1)});
      break;
    case Parser::Nodes::BinaryOperationType::LessEqual:
      result += instruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      result += instruction("mov", {resultLocation.asAsm(8), "0"});
      result += instruction("setle", {resultLocation.asAsm(1)});
      break;
    case Parser::Nodes::BinaryOperationType::LessThan:
      result += instruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      result += instruction("mov", {resultLocation.asAsm(8), "0"});
      result += instruction("setl", {resultLocation.asAsm(1)});
      break;
    default:
      throw GenerationException("Unhandled binary operation");
  }

  return result;
}

std::string Generator::genUnaryOperation(
    const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) const {
  const auto* unaryNode = dynamic_cast<const Parser::Nodes::UnaryOperation*>(node);
  std::string result = genExpression(unaryNode->operand(), resultLocation, context);

  switch (unaryNode->operation()) {
    case Parser::Nodes::UnaryOperationType::Negate:
      result += instruction("neg", {resultLocation.asAsm(8)});
      break;
    case Parser::Nodes::UnaryOperationType::Not:
      result += instruction("not", {resultLocation.asAsm(8)});
      break;
    case Parser::Nodes::UnaryOperationType::Decrement:
      result += instruction("dec", {resultLocation.asAsm(8)});
      break;
    case Parser::Nodes::UnaryOperationType::Increment:
      result += instruction("inc", {resultLocation.asAsm(8)});
      break;
    default:
      throw GenerationException("Unhandled unary operation");
  }

  return result;
}

std::string Generator::genBaseOperation(
    const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) const {
  switch (node->type()) {
    case NodeType::IntegerLiteral:
      return instruction("mov", {resultLocation.asAsm(8), node->token()->value.value()});
    case NodeType::Identifier: {
      const std::string identName = node->token()->value.value();
      const std::optional<Context::VariableContext> varContext = context.variable(identName);

      if (!varContext) {
        throw GenerationException("Unknown identifier in expression");
      }
      const MemoryLocation location{"[ rsp + " + std::to_string(varContext->offset) + " ]"};
      std::string ret = instruction("mov", {resultLocation.asAsm(varContext->size), location.asAsm(varContext->size)});
      if (varContext->size != 8) {
        const std::string_view inst = varContext->size == 1 ? "movsx" : "movsxd";
        ret += instruction(inst, {resultLocation.asAsm(8), resultLocation.asAsm(varContext->size)});
      }
      return ret;
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

const Register& Generator::nextRegister(const Location& location) {
  const auto* reg = dynamic_cast<const Register*>(&location);
  if (!reg) {
    // We got a memory address, so give the first register
    return REGISTERS[0];
  }
  // Find the next one
  auto current = std::ranges::find(REGISTERS, *reg);
  if (current == REGISTERS.end() || current++ == REGISTERS.end()) {
    throw GenerationException("Unable to get next register");
  }
  return *current++;
}
