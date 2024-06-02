#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace Cepheid::Parser::Nodes {
class VariableDeclaration;
}

namespace Cepheid::Gen {
class Context {
 public:
  Context();
  Context(const Context& other) = delete;
  Context(Context&& other) noexcept = delete;
  Context& operator=(const Context& other) = delete;
  Context& operator=(Context&& other) noexcept = delete;
  ~Context() = default;

  void pushScope();
  void pushFunction();

  void popScope();
  void popFunction();

  struct VariableContext {
    size_t size;
    size_t offset;
  };

  void addVariable(const Parser::Nodes::VariableDeclaration* variable);
  [[nodiscard]] std::optional<VariableContext> variable(const std::string& name) const;

  struct TypeContext {
    size_t size;
    size_t alignment;
  };

  [[nodiscard]] std::optional<TypeContext> type(const std::string& name) const;

  [[nodiscard]] size_t nextLocalLabel();

 private:
  struct ContextImpl {
    std::unique_ptr<ContextImpl> parent;

    size_t stackOffset = 0;

    std::map<std::string, VariableContext> variables;
    std::map<std::string, TypeContext> types;
  };

  template <typename ReturnT>
  [[nodiscard]] std::optional<ReturnT> recurseContext(
      const ContextImpl& context, std::function<std::optional<ReturnT>(const ContextImpl&)>) const;

  std::unique_ptr<ContextImpl> m_impl;
  size_t m_localLabels = 0;

  std::map<std::string, TypeContext> m_primitiveTypes;
};

}  // namespace Cepheid::Gen
