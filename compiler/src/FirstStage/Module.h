#pragma once

#include <map>
#include <Parser/Parser.h>

#include <FirstStage/Function.h>
#include <FirstStage/Type.h>

namespace Cepheid::Eval {

class Module {
 public:
  explicit Module(std::string_view name);
  explicit Module(const Parser::Nodes::Node& moduleNode, const Module* parent);

  const Type* type(const std::string& name) const;

  void evaluate();

 private:
  void add(const Parser::Nodes::Node& node);
  void addModule(const Parser::Nodes::Node& node);
  void addFunction(const Parser::Nodes::Node& node);

  const Module* m_parent;
  std::string m_name;
  std::map<std::string, Module> m_modules;
  std::map<std::string, Function> m_functions;
  std::map<std::string, Type> m_types;
};

}  // namespace Cepheid::Eval
