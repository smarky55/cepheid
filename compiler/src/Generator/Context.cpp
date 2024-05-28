#include "Context.h"

#include <Parser/Node/Scope.h>
#include <ranges>
#include <Parser/Node/VariableDeclaration.h>

using namespace Cepheid::Gen;

Context::Context() : m_impl(std::make_unique<ContextImpl>()) {
}

void Context::push() {
  auto newContext = std::make_unique<ContextImpl>();
  newContext->m_stackOffset = m_impl->m_stackOffset;
  for (const auto& [size, offset] : m_impl->m_variables | std::views::values) {
    newContext->m_stackOffset += size;
  }
  newContext->m_parent = std::move(m_impl);
  m_impl = std::move(newContext);
}

void Context::pop() {
  m_impl = std::move(m_impl->m_parent);
}

void Context::addVariable(const Parser::Nodes::VariableDeclaration* variable) {
  VariableContext varContext;
  varContext.m_offset = m_impl->m_stackOffset;
  for (const auto& [size, offset] : m_impl->m_variables | std::views::values) {
    varContext.m_offset += size;
  }
  varContext.m_size = 8; // TODO: Not everything is 8 bytes
  m_impl->m_variables.try_emplace(variable->name(), varContext);
}

std::optional<Context::VariableContext> Context::variable(const std::string& name) const {
  auto func = [name](const ContextImpl& context) -> std::optional<VariableContext> {
    if (context.m_variables.contains(name)) {
      return context.m_variables.at(name);
    }
    return std::nullopt;
  };

  return recurseContext<VariableContext>(*m_impl, func);
}

template <typename ReturnT>
std::optional<ReturnT> Context::recurseContext(
    const ContextImpl& context, std::function<std::optional<ReturnT>(const Context::ContextImpl&)> findFunc) const {
  if (auto ret = findFunc(context)) {
    return ret;
  }

  if (context.m_parent) return recurseContext(*(context.m_parent), findFunc);

  return {};
}
