#include "Generator.h"

#include <Generator/Context.h>
#include <Generator/GenerationException.h>
#include <Generator/Location/Location.h>
#include <Generator/Location/MemoryLocation.h>
#include <Generator/Location/Register.h>
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

std::string Generator::generate() {
  Context context;
  genProgram(m_root.get(), context);

  return m_program.str();
}

void Generator::genProgram(const Parser::Nodes::Node* node, Context& context) {
  m_program << R"(bits 64
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
    genStatement(child.get(), context);
  }
}

void Generator::genScope(const Parser::Nodes::Scope* scope, Context& context) {
  context.pushScope();

  if (!scope) {
    throw GenerationException("Expected scope!");
  }

  for (const auto& child : scope->statements()) {
    genStatement(child.get(), context);
  }

  context.popScope();
}

void Generator::genStatement(const Parser::Nodes::Node* node, Context& context) {
  switch (node->type()) {
    case NodeType::Function:
      genFunction(node, context);
      break;
    case NodeType::ReturnStatement:
      genReturn(node, context);
      break;
    case NodeType::VariableDeclaration:
      genVariableDeclaration(node, context);
      break;
    case NodeType::Conditional:
      genConditional(node, context);
      break;
    case NodeType::Loop:
      // genLoop(node, context);
      break;
    case NodeType::Expression:
      // TODO: how to handle expression statements?
      break;
    default:
      break;
  }
}

void Generator::genFunction(const Parser::Nodes::Node* node, Context& context) {
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
  writeLabel(name);
  writeInstruction("push", {"rbp"});
  writeInstruction("mov", {"rbp", "rsp"});

  writeInstruction("sub", {"rsp", std::to_string(stackSpace)});

  // Scope
  genScope(function->scope(), context);

  context.popFunction();
}

void Generator::genReturn(const Parser::Nodes::Node* node, Context& context) {
  std::string ret;
  // Compute expression
  if (!node->children().empty()) {
    genExpression(node->children()[0].get(), REGISTERS[1], context);
    writeInstruction("mov", {"rax", "rbx"});
  }

  // Do the return
  writeInstruction("leave", {});
  writeInstruction("ret", {});
}

void Generator::genVariableDeclaration(const Parser::Nodes::Node* node, Context& context) {
  const auto variableDeclaration = dynamic_cast<const Parser::Nodes::VariableDeclaration*>(node);
  if (!variableDeclaration) {
    throw GenerationException("Expected variable declaration");
  }

  context.addVariable(variableDeclaration);

  const std::optional<Context::VariableContext> varContext = context.variable(variableDeclaration->name());
  if (!varContext) {
    throw GenerationException("Somehow failed to add variable to context");
  }

  if (variableDeclaration->expression()) {
    const MemoryLocation location{"[ rsp + " + std::to_string(varContext->offset) + " ]"};
    const Register& reg = nextRegister(location);
    genExpression(variableDeclaration->expression(), reg, context);
    writeInstruction("mov", {location.asAsm(varContext->size), reg.asAsm(varContext->size)});
  }
}

void Generator::genConditional(const Parser::Nodes::Node* node, Context& context) {
  const auto conditional = dynamic_cast<const Parser::Nodes::Conditional*>(node);
  if (!conditional) {
    throw GenerationException("Expected conditional");
  }
  const size_t labelIndex = context.nextLocalLabel();
  const std::string label = ".L" + std::to_string(labelIndex);
  const Register& conditionRegister = REGISTERS[0];
  genExpression(conditional->expression(), conditionRegister, context);
  writeInstruction("cmp", {conditionRegister.asAsm(1), "0"});
  writeInstruction("je", {label});
  genScope(conditional->scope(), context);
  writeLabel(label);
}

void Generator::genExpression(const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) {
  switch (node->type()) {
    case NodeType::BinaryOperation:
      genBinaryOperation(node, resultLocation, context);
      break;
    case NodeType::UnaryOperation:
      genUnaryOperation(node, resultLocation, context);
      break;
    default:
      genBaseOperation(node, resultLocation, context);
      break;
  }
}

void Generator::genBinaryOperation(const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) {
  const auto* binaryNode = dynamic_cast<const Parser::Nodes::BinaryOperation*>(node);
  const Register& rhsReg = nextRegister(resultLocation);
  genExpression(binaryNode->lhs(), resultLocation, context);
  genExpression(binaryNode->rhs(), rhsReg, context);

  switch (binaryNode->operation()) {
    case Parser::Nodes::BinaryOperationType::Add:
      writeInstruction("add", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      break;
    case Parser::Nodes::BinaryOperationType::Subtract:
      writeInstruction("sub", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      break;
    case Parser::Nodes::BinaryOperationType::Multiply:
      writeInstruction("imul", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      break;
    case Parser::Nodes::BinaryOperationType::Divide:
      // TODO: IDIV needs its LHS in RDX:RAX first and stores the Quotient in RAX and remainder in RDX
      // instruction("idiv", {resultReg, rhsReg});
      break;
    case Parser::Nodes::BinaryOperationType::Equal:
      writeInstruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      writeInstruction("mov", {resultLocation.asAsm(8), "0"});
      writeInstruction("sete", {resultLocation.asAsm(1)});
      break;
    case Parser::Nodes::BinaryOperationType::NotEqual:
      writeInstruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      writeInstruction("mov", {resultLocation.asAsm(8), "0"});
      writeInstruction("setne", {resultLocation.asAsm(1)});
      break;
    case Parser::Nodes::BinaryOperationType::GreaterEqual:
      writeInstruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      writeInstruction("mov", {resultLocation.asAsm(8), "0"});
      writeInstruction("setge", {resultLocation.asAsm(1)});
      break;
    case Parser::Nodes::BinaryOperationType::GreaterThan:
      writeInstruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      writeInstruction("mov", {resultLocation.asAsm(8), "0"});
      writeInstruction("setg", {resultLocation.asAsm(1)});
      break;
    case Parser::Nodes::BinaryOperationType::LessEqual:
      writeInstruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      writeInstruction("mov", {resultLocation.asAsm(8), "0"});
      writeInstruction("setle", {resultLocation.asAsm(1)});
      break;
    case Parser::Nodes::BinaryOperationType::LessThan:
      writeInstruction("cmp", {resultLocation.asAsm(8), rhsReg.asAsm(8)});
      writeInstruction("mov", {resultLocation.asAsm(8), "0"});
      writeInstruction("setl", {resultLocation.asAsm(1)});
      break;
    default:
      throw GenerationException("Unhandled binary operation");
  }
}

void Generator::genUnaryOperation(const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) {
  const auto* unaryNode = dynamic_cast<const Parser::Nodes::UnaryOperation*>(node);
  genExpression(unaryNode->operand(), resultLocation, context);

  switch (unaryNode->operation()) {
    case Parser::Nodes::UnaryOperationType::Negate:
      writeInstruction("neg", {resultLocation.asAsm(8)});
      break;
    case Parser::Nodes::UnaryOperationType::Not:
      writeInstruction("not", {resultLocation.asAsm(8)});
      break;
    case Parser::Nodes::UnaryOperationType::Decrement:
      writeInstruction("dec", {resultLocation.asAsm(8)});
      break;
    case Parser::Nodes::UnaryOperationType::Increment:
      writeInstruction("inc", {resultLocation.asAsm(8)});
      break;
    default:
      throw GenerationException("Unhandled unary operation");
  }
}

void Generator::genBaseOperation(const Parser::Nodes::Node* node, const Location& resultLocation, Context& context) {
  switch (node->type()) {
    case NodeType::IntegerLiteral:
      writeInstruction("mov", {resultLocation.asAsm(8), node->token()->value.value()});
      break;
    case NodeType::Identifier: {
      const std::string identName = node->token()->value.value();
      const std::optional<Context::VariableContext> varContext = context.variable(identName);

      if (!varContext) {
        throw GenerationException("Unknown identifier in expression");
      }
      const MemoryLocation location{"[ rsp + " + std::to_string(varContext->offset) + " ]"};
      writeInstruction("mov", {resultLocation.asAsm(varContext->size), location.asAsm(varContext->size)});
      if (varContext->size != 8) {
        const std::string_view inst = varContext->size == 1 ? "movsx" : "movsxd";
        writeInstruction(inst, {resultLocation.asAsm(8), resultLocation.asAsm(varContext->size)});
      }
      break;
    }
    default:
      break;
  }
}

void Generator::writeInstruction(std::string_view inst, const std::vector<std::string_view>& args) {
  m_program << "  " << inst;
  if (!args.empty()) {
    m_program << " " << args[0];
    for (size_t i = 1; i < args.size(); i++) {
      m_program << ", " << args[i];
    }
  }
  m_program << "\n";
}

void Generator::writeLabel(std::string_view label) {
  m_program << label << ":\n";
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
