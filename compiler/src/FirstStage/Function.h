#pragma once

#include <FirstStage/EvaluationContext.h>
#include <FirstStage/Type.h>
#include <FirstStage/Statements/Statement.h>

#include <Parser/Node/Function.h>

#include <vector>

namespace Cepheid::Eval {
class Module;
class Value;

class Function {
 public:
  explicit Function(const Parser::Nodes::Function& funcNode, const Module* parent);

 private:
  void addStatement(const Parser::Nodes::Node& node);
  void addScope(const Parser::Nodes::Node& node);
  void addVariableDeclaration(const Parser::Nodes::Node& node);
  void addReturn(const Parser::Nodes::Node& node);
  void addConditional(const Parser::Nodes::Node& node);
  void addLoop(const Parser::Nodes::Node& node);
  std::shared_ptr<Value> addExpression(const Parser::Nodes::Node& node);
  std::shared_ptr<Value> addBinaryExpression(const Parser::Nodes::Node& node);
  std::shared_ptr<Value> addUnaryExpression(const Parser::Nodes::Node& node);
  std::shared_ptr<Value> getExpressionBase(const Parser::Nodes::Node& node);
  std::shared_ptr<Value> getTemporary(const Type* type);

  template<typename StatementT, typename... ArgsT>
  void emplaceStatement(ArgsT&&... args);

  const Module* m_parent;
  std::string m_name;
  //const Type* m_returns = nullptr;
  //std::vector<const Type*> m_parameters;
  std::vector<std::unique_ptr<Statement>> m_statements;
  std::map<std::string, const Type*> m_variables;
  int m_temporaryIndex = 1;
  int m_labelIndex = 0;
  EvaluationContext m_evaluationContext;
};

}  // namespace Cepheid::Eval
