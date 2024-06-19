#include "Generator.h"

#include <Generator/Context.h>
#include <Generator/GenerationException.h>
#include <Generator/Location/Location.h>
#include <Generator/Location/MemoryLocation.h>
#include <Generator/Location/Register.h>
#include <Parser/Node/BinaryOperation.h>
#include <Parser/Node/Conditional.h>
#include <Parser/Node/Function.h>
#include <Parser/Node/Loop.h>
#include <Parser/Node/Scope.h>
#include <Parser/Node/UnaryOperation.h>
#include <Parser/Node/VariableDeclaration.h>
#include <Tokeniser/Token.h>

#include <array>
#include <optional>
#include <sstream>
#include <Generator/Location/Comparison.h>
#include <Generator/Location/IntegerLiteral.h>

using namespace Cepheid::Gen;

using Cepheid::Parser::Nodes::NodeType;

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
      genLoop(node, context);
      break;
    case NodeType::Expression:
      genExpression(node, context);
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
    const std::unique_ptr<Location> resultLocation = genExpression(node->children()[0].get(), context);
    writeInstruction("mov", {"rax", resultLocation->asAsm(8)});
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
    std::unique_ptr<Location> resultLocation = genExpression(variableDeclaration->expression(), context);
    resultLocation = writeImmediateToReg(std::move(resultLocation), context);
    writeInstruction("mov", {location.asAsm(varContext->size), resultLocation->asAsm(varContext->size)});
  }
}

void Generator::genConditional(const Parser::Nodes::Node* node, Context& context) {
  const auto conditional = dynamic_cast<const Parser::Nodes::Conditional*>(node);
  if (!conditional) {
    throw GenerationException("Expected conditional");
  }
  const size_t labelIndex = context.nextLocalLabel();
  const std::string label = ".L" + std::to_string(labelIndex);

  const std::unique_ptr<Location> resultLocation = genExpression(conditional->expression(), context);
  std::unique_ptr<Comparison> comparison;
  if (const auto resultComparison = dynamic_cast<Comparison*>(resultLocation.get())) {
    comparison = std::make_unique<Comparison>(*resultComparison);
  } else {
    writeInstruction("cmp", {resultLocation->asAsm(8), "0"});
    comparison = std::make_unique<Comparison>(Comparison::Type::Equal);
  }
  writeInstruction(comparison->jmpInstruction(true), {label});
  genScope(conditional->scope(), context);
  writeLabel(label);
}

void Generator::genLoop(const Parser::Nodes::Node* node, Context& context) {
  const auto loop = dynamic_cast<const Parser::Nodes::Loop*>(node);
  if (!loop) {
    throw GenerationException("Expected loop");
  }

  if (const Parser::Nodes::Node* initExpression = loop->initExpression()) {
    genExpression(initExpression, context);
  }

  const std::string conditionLabel = ".L" + std::to_string(context.nextLocalLabel());
  const std::string endLabel = ".L" + std::to_string(context.nextLocalLabel());

  writeLabel(conditionLabel);
  const std::unique_ptr<Location> conditionLocation = genExpression(loop->conditionExpression(), context);
  std::unique_ptr<Comparison> comparison;
  if (const auto resultComparison = dynamic_cast<Comparison*>(conditionLocation.get())) {
    comparison = std::make_unique<Comparison>(*resultComparison);
  } else {
    writeInstruction("cmp", {conditionLocation->asAsm(8), "0"});
    comparison = std::make_unique<Comparison>(Comparison::Type::Equal);
  }
  writeInstruction(comparison->jmpInstruction(true), {endLabel});

  genScope(loop->scope(), context);

  if (const Parser::Nodes::Node* updateExpression = loop->updateExpression()) {
    genExpression(updateExpression, context);
  }

  writeInstruction("jmp", {conditionLabel});
  writeLabel(endLabel);
}

std::unique_ptr<Location> Generator::genExpression(const Parser::Nodes::Node* node, Context& context) {
  if (node->type() == NodeType::Expression) {
    return genExpression(node->children().front().get(), context);
  }
  switch (node->type()) {
    case NodeType::BinaryOperation:
      return genBinaryOperation(node, context);
    case NodeType::UnaryOperation:
      return genUnaryOperation(node, context);
    default:
      return genBaseOperation(node, context);
  }
}

std::unique_ptr<Location> Generator::genBinaryOperation(const Parser::Nodes::Node* node, Context& context) {
  const auto* binaryNode = dynamic_cast<const Parser::Nodes::BinaryOperation*>(node);
  std::unique_ptr<Location> lhsLoc = genExpression(binaryNode->lhs(), context);
  std::unique_ptr<Location> rhsLoc = genExpression(binaryNode->rhs(), context);

  lhsLoc = writeComparisonToReg(std::move(lhsLoc), context);
  lhsLoc = writeImmediateToReg(std::move(lhsLoc), context);
  rhsLoc = writeComparisonToReg(std::move(rhsLoc), context);

  switch (binaryNode->operation()) {
    case Parser::Nodes::BinaryOperationType::Add:
      writeInstruction("add", {lhsLoc->asAsm(8), rhsLoc->asAsm(8)});
      break;
    case Parser::Nodes::BinaryOperationType::Subtract:
      writeInstruction("sub", {lhsLoc->asAsm(8), rhsLoc->asAsm(8)});
      break;
    case Parser::Nodes::BinaryOperationType::Multiply:
      writeInstruction("imul", {lhsLoc->asAsm(8), rhsLoc->asAsm(8)});
      break;
    case Parser::Nodes::BinaryOperationType::Divide:
      // TODO: IDIV needs its LHS in RDX:RAX first and stores the Quotient in RAX and remainder in RDX
      // instruction("idiv", {resultReg, rhsReg});
      break;
    case Parser::Nodes::BinaryOperationType::Equal:
      rhsLoc = writeImmediateToReg(std::move(rhsLoc), context);
      writeInstruction("cmp", {lhsLoc->asAsm(8), rhsLoc->asAsm(8)});
      return std::make_unique<Comparison>(Comparison::Type::Equal);
    case Parser::Nodes::BinaryOperationType::NotEqual:
      rhsLoc = writeImmediateToReg(std::move(rhsLoc), context);
      writeInstruction("cmp", {lhsLoc->asAsm(8), rhsLoc->asAsm(8)});
      return std::make_unique<Comparison>(Comparison::Type::NotEqual);
    case Parser::Nodes::BinaryOperationType::GreaterEqual:
      rhsLoc = writeImmediateToReg(std::move(rhsLoc), context);
      writeInstruction("cmp", {lhsLoc->asAsm(8), rhsLoc->asAsm(8)});
      return std::make_unique<Comparison>(Comparison::Type::GreaterEqual);
    case Parser::Nodes::BinaryOperationType::GreaterThan:
      rhsLoc = writeImmediateToReg(std::move(rhsLoc), context);
      writeInstruction("cmp", {lhsLoc->asAsm(8), rhsLoc->asAsm(8)});
      return std::make_unique<Comparison>(Comparison::Type::Greater);
    case Parser::Nodes::BinaryOperationType::LessEqual:
      rhsLoc = writeImmediateToReg(std::move(rhsLoc), context);
      writeInstruction("cmp", {lhsLoc->asAsm(8), rhsLoc->asAsm(8)});
      return std::make_unique<Comparison>(Comparison::Type::LessEqual);
    case Parser::Nodes::BinaryOperationType::LessThan:
      rhsLoc = writeImmediateToReg(std::move(rhsLoc), context);
      writeInstruction("cmp", {lhsLoc->asAsm(8), rhsLoc->asAsm(8)});
      return std::make_unique<Comparison>(Comparison::Type::Less);
    case Parser::Nodes::BinaryOperationType::Assign:
      // TODO: Assignment
      break;
    default:
      throw GenerationException("Unhandled binary operation");
  }
  return lhsLoc;
}

std::unique_ptr<Location> Generator::genUnaryOperation(const Parser::Nodes::Node* node, Context& context) {
  const auto* unaryNode = dynamic_cast<const Parser::Nodes::UnaryOperation*>(node);
  std::unique_ptr<Location> resultLocation = genExpression(unaryNode->operand(), context);
  resultLocation = writeComparisonToReg(std::move(resultLocation), context);
  resultLocation = writeImmediateToReg(std::move(resultLocation), context);

  switch (unaryNode->operation()) {
    case Parser::Nodes::UnaryOperationType::Negate:
      writeInstruction("neg", {resultLocation->asAsm(8)});
      break;
    case Parser::Nodes::UnaryOperationType::Not:
      writeInstruction("not", {resultLocation->asAsm(8)});
      break;
    case Parser::Nodes::UnaryOperationType::Decrement:
      writeInstruction("dec", {resultLocation->asAsm(8)});
      break;
    case Parser::Nodes::UnaryOperationType::Increment:
      writeInstruction("inc", {resultLocation->asAsm(8)});
      break;
    default:
      throw GenerationException("Unhandled unary operation");
  }
  return resultLocation;
}

std::unique_ptr<Location> Generator::genBaseOperation(const Parser::Nodes::Node* node, Context& context) {
  switch (node->type()) {
    case NodeType::IntegerLiteral:
      return std::make_unique<IntegerLiteral>(node->token()->value.value());
      break;
    case NodeType::Identifier: {
      const std::string identName = node->token()->value.value();
      const std::optional<Context::VariableContext> varContext = context.variable(identName);

      if (!varContext) {
        throw GenerationException("Unknown identifier in expression");
      }
      return std::make_unique<MemoryLocation>("[ rsp + " + std::to_string(varContext->offset) + " ]");
    }
    default:
      throw GenerationException("Unhandled expression");
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

std::unique_ptr<Location> Generator::writeComparisonToReg(std::unique_ptr<Location> loc, Context& context) {
  if (const auto comparison = dynamic_cast<Comparison*>(loc.get())) {
    std::unique_ptr<Location> resultLocation = context.nextRegister();

    writeInstruction("mov", {resultLocation->asAsm(8), "0"});
    writeInstruction(comparison->setInstruction(), {resultLocation->asAsm(1)});

    return resultLocation;
  }
  return loc;
}

std::unique_ptr<Location> Generator::writeImmediateToReg(std::unique_ptr<Location> loc, Context& context) {
  if (const auto literal = dynamic_cast<IntegerLiteral*>(loc.get())) {
    std::unique_ptr<Location> resultLocation = context.nextRegister();

    writeInstruction("mov", {resultLocation->asAsm(8), literal->asAsm(8)});

    return resultLocation;
  }
  return loc;
}
