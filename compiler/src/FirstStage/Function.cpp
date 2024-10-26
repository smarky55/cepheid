#include "Function.h"

#include <format>
#include <FirstStage/EvaluationException.h>
#include <FirstStage/LiteralValue.h>
#include <FirstStage/Module.h>
#include <FirstStage/Type.h>
#include <FirstStage/Value.h>
#include <FirstStage/VariableValue.h>
#include <FirstStage/Statements/BinaryOperation.h>
#include <FirstStage/Statements/JumpStatement.h>
#include <FirstStage/Statements/Label.h>
#include <FirstStage/Statements/ReturnStatement.h>
#include <FirstStage/Statements/UnaryOperation.h>
#include <nlohmann/thirdparty/hedley/hedley.hpp>
#include <Parser/Node/BinaryOperation.h>
#include <Parser/Node/Conditional.h>
#include <Parser/Node/Loop.h>
#include <Parser/Node/Scope.h>
#include <Parser/Node/UnaryOperation.h>
#include <Parser/Node/VariableDeclaration.h>

Cepheid::Eval::Function::Function(const Parser::Nodes::Function& funcNode, const Module* parent)
    : m_parent(parent), m_name(funcNode.name()) {
  // TODO: add function parameters
  addScope(*funcNode.scope());
}

void Cepheid::Eval::Function::addStatement(const Parser::Nodes::Node& node) {
  switch (node.type()) {
    case Parser::Nodes::NodeType::Scope:
      addScope(node);
      break;
    case Parser::Nodes::NodeType::VariableDeclaration:
      addVariableDeclaration(node);
      break;
    case Parser::Nodes::NodeType::Expression:
      addExpression(node);
      break;
    case Parser::Nodes::NodeType::ReturnStatement:
      addReturn(node);
      break;
    case Parser::Nodes::NodeType::Conditional:
      addConditional(node);
      break;
    case Parser::Nodes::NodeType::Loop:
      addLoop(node);
      break;
  }
}

void Cepheid::Eval::Function::addScope(const Parser::Nodes::Node& node) {
  const auto* scopeNode = dynamic_cast<const Parser::Nodes::Scope*>(&node);
  if (!scopeNode) {
    throw EvaluationException("Invalid node type for scope");
  }
  m_evaluationContext.push();
  for (const auto& childNode : scopeNode->statements()) {
    addStatement(*childNode);
  }
  m_evaluationContext.pop();
}

void Cepheid::Eval::Function::addVariableDeclaration(const Parser::Nodes::Node& node) {
  const auto* variableNode = dynamic_cast<const Parser::Nodes::VariableDeclaration*>(&node);
  if (!variableNode) {
    throw EvaluationException("Invalid node type for variable declaration");
  }
  // TODO: Variable declaration
  // Make a note of the variable
  const Type* type = m_parent->type(*variableNode->typeName()->token()->value);
  if (const auto& [it, success] = m_variables.try_emplace(variableNode->name(), type); !success) {
    throw EvaluationException("Variable already defined");
  }
  // Allocate a space

  auto destination = addExpression(*variableNode->expression());

  // Assign to variable?
}

void Cepheid::Eval::Function::addReturn(const Parser::Nodes::Node& node) {
  std::shared_ptr<Value> returnValue;
  if (!node.children().empty()) {
    returnValue = addExpression(*node.children()[0]);
  }

  emplaceStatement<ReturnStatement>(returnValue);
}

void Cepheid::Eval::Function::addConditional(const Parser::Nodes::Node& node) {
  const auto conditional = dynamic_cast<const Parser::Nodes::Conditional*>(&node);

  std::shared_ptr<Value> comparisonResult = addExpression(*conditional->expression());

  const std::string labelName = std::format(".L{}", m_labelIndex++);
  emplaceStatement<JumpStatement>(labelName, JumpStatement::JumpIf::False, comparisonResult);
  addScope(*conditional->scope());
  emplaceStatement<Label>(labelName);
}

void Cepheid::Eval::Function::addLoop(const Parser::Nodes::Node& node) {
  const auto* loopNode = dynamic_cast<const Parser::Nodes::Loop*>(&node);

  // Push scope?

  // Do loop initialisation
  if (const auto* initExpressionNode = loopNode->initExpression()) {
    addStatement(*initExpressionNode);
  }

  const int labelIndex = m_labelIndex++;
  const std::string startLabelName = std::format(".L{}_start", labelIndex);
  const std::string conditionLabelName = std::format(".L{}_cond", labelIndex);

  // Unconditional jump to check condition
  emplaceStatement<JumpStatement>(conditionLabelName, JumpStatement::JumpIf::Always);

  // Do loop scope
  emplaceStatement<Label>(startLabelName);
  addScope(*loopNode->scope());

  // Do loop update
  if (const auto* updateExpressionNode = loopNode->updateExpression()) {
    addExpression(*updateExpressionNode);
  }

  // Check condition
  emplaceStatement<Label>(conditionLabelName);
  std::shared_ptr<Value> conditionResult = addExpression(*loopNode->conditionExpression());
  emplaceStatement<JumpStatement>(startLabelName, JumpStatement::JumpIf::True, conditionResult);

  // Pop scope?
}

std::shared_ptr<Cepheid::Eval::Value> Cepheid::Eval::Function::addExpression(const Parser::Nodes::Node& node) {
  switch (node.type()) {
    case Parser::Nodes::NodeType::Expression:
      return addExpression(*node.children().front());
    case Parser::Nodes::NodeType::BinaryOperation:
      return addBinaryExpression(node);
    case Parser::Nodes::NodeType::UnaryOperation:
      return addUnaryExpression(node);
    default:
      return getExpressionBase(node);
  }
}

std::shared_ptr<Cepheid::Eval::Value> Cepheid::Eval::Function::addBinaryExpression(const Parser::Nodes::Node& node) {
  const auto* binaryNode = dynamic_cast<const Parser::Nodes::BinaryOperation*>(&node);
  std::shared_ptr<Value> lhs = addExpression(*binaryNode->lhs());
  std::shared_ptr<Value> rhs = addExpression(*binaryNode->rhs());

  // TODO: binary expression
  //  convert if needed
  //  Get our result location (might be an assignment?)
  std::shared_ptr<Value> result;
  if (binaryNode->operation() == Parser::Nodes::BinaryOperationType::Assign) {
    result = lhs;
  } else {
    result = getTemporary(lhs->type());
  }

  //  Add this expression
  emplaceStatement<BinaryOperation>(binaryNode->operation(), result, lhs, rhs);

  //  Return our result location
  return result;
}

std::shared_ptr<Cepheid::Eval::Value> Cepheid::Eval::Function::addUnaryExpression(const Parser::Nodes::Node& node) {
  const auto* unaryNode = dynamic_cast<const Parser::Nodes::UnaryOperation*>(&node);
  const std::shared_ptr<Value> operand = addExpression(*unaryNode->operand());

  // Get our temporary location
  std::shared_ptr<Value> result = getTemporary(operand->type());
  // Add this expression
  emplaceStatement<UnaryOperation>(unaryNode->operation(), result, operand);
  // Return our temporary location
  return result;
}

std::shared_ptr<Cepheid::Eval::Value> Cepheid::Eval::Function::getExpressionBase(const Parser::Nodes::Node& node) {
  switch (node.type()) {
    case Parser::Nodes::NodeType::IntegerLiteral: {
      if (!node.token()) {
        throw EvaluationException("Invalid token");
      }
      const auto literalVal = node.token()->value;
      if (!literalVal) {
        throw EvaluationException("Invalid literal value");
      }
      return std::make_shared<LiteralValue<int32_t>>(m_parent->type("i32"), std::stoi(*literalVal));
    }
    case Parser::Nodes::NodeType::Identifier: {
      const auto it = m_variables.find(*node.token()->value);
      if (it == m_variables.end()) {
        throw EvaluationException("Unknown identifier");
      }
      return std::make_shared<VariableValue>(it->second, it->first);
    }
  }
  throw EvaluationException("Unhandled value in expression");
}

std::shared_ptr<Cepheid::Eval::Value> Cepheid::Eval::Function::getTemporary(const Type* type) {
  const auto& [it, success] = m_variables.emplace(std::to_string(m_temporaryIndex++), type);
  return std::make_shared<VariableValue>(it->second, it->first);
}

template <typename StatementT, typename... ArgsT>
void Cepheid::Eval::Function::emplaceStatement(ArgsT&&... args) {
  m_statements.emplace_back(std::make_unique<StatementT>(std::forward<ArgsT>(args)...));
}
