#pragma once

#include <Generator/Location/Register.h>

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
  struct VariableContext {
    size_t size;
    size_t offset;
  };

  struct TypeContext {
    size_t size;
    size_t alignment;
  };

  struct RegisterContext {
    RegisterContext(Register reg);
    Register reg;
    bool inUse = false;
  };

  struct RegisterHandle : Location{
    [[nodiscard]] explicit RegisterHandle(RegisterContext* context);
    ~RegisterHandle() override;
    [[nodiscard]] const Register& reg() const;
    [[nodiscard]] std::string asAsm(size_t size) const override;

   private:
    RegisterContext* m_context;
  };

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

  void addVariable(const Parser::Nodes::VariableDeclaration* variable);
  [[nodiscard]] std::optional<VariableContext> variable(const std::string& name) const;

  [[nodiscard]] std::optional<TypeContext> type(const std::string& name) const;

  [[nodiscard]] size_t nextLocalLabel();

  [[nodiscard]] std::unique_ptr<Context::RegisterHandle> nextRegister();

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

  std::vector<RegisterContext> m_registers;

  std::map<std::string, TypeContext> m_primitiveTypes;
};

}  // namespace Cepheid::Gen
