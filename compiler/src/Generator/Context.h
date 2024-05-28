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

  void push();

  void pop();

  struct VariableContext {
    size_t m_size;
    size_t m_offset;
  };

  void addVariable(const Parser::Nodes::VariableDeclaration* variable);
  std::optional<VariableContext> variable(const std::string& name) const;

 private:
  struct ContextImpl {
    std::unique_ptr<ContextImpl> m_parent;

    size_t m_stackOffset = 0;

    std::map<std::string, VariableContext> m_variables;
  };

  template <typename ReturnT>
  [[nodiscard]] std::optional<ReturnT> recurseContext(
      const ContextImpl& context, std::function<std::optional<ReturnT>(const ContextImpl&)>) const;

  std::unique_ptr<ContextImpl> m_impl;
};

}  // namespace Cepheid::Gen
