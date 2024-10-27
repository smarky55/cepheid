#include "Module.h"

#include <Parser/Node/Function.h>

#include <FirstStage/EvaluationException.h>
#include <FirstStage/Function.h>

#include <ranges>

using namespace Cepheid::Eval;

Module::Module(const Parser::Nodes::Node& moduleNode, const Module* parent) : m_parent(parent) {
  for (const auto& child : moduleNode.children()) {
    add(*child);
  }

  if (!parent) {
    initAsRoot();
  }
}

const Type* Module::type(const std::string& name) const {
  if (m_types.contains(name)) {
    return &m_types.at(name);
  }
  return m_parent ? m_parent->type(name) : nullptr;
}

void Module::evaluate() {
  for (auto& function : m_functions | std::views::values) {
    function.evaluate();
  }
}

void Module::add(const Parser::Nodes::Node& node) {
  switch (node.type()) {
    case Parser::Nodes::NodeType::Module:
      addModule(node);
      break;
    case Parser::Nodes::NodeType::Function:
      addFunction(node);
      break;
    default:
      throw EvaluationException("Unexpected node in module");
  }
}

void Module::addModule(const Cepheid::Parser::Nodes::Node& node) {
  throw EvaluationException("Not Implemented: Module::addModule()");
}

void Module::addFunction(const Parser::Nodes::Node& node) {
  const auto& functionNode = dynamic_cast<const Parser::Nodes::Function&>(node);
  m_functions.try_emplace(functionNode.name(), functionNode, this);
}

void Module::addType(std::string_view name, std::size_t size, std::size_t alignment) {
  m_types.try_emplace(std::string(name), name, size, alignment);
}

void Module::initAsRoot() {
  addType("i8", 1, 1);
  addType("i16",2, 2);
  addType("i32", 4, 4);
  addType("i64", 8, 8);
}