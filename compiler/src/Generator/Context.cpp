#include "Context.h"

#include <Generator/GenerationException.h>
#include <Parser/Node/VariableDeclaration.h>

#include <ranges>

using namespace Cepheid::Gen;

Context::RegisterContext::RegisterContext(Register reg) : reg(std::move(reg)) {
}

Context::RegisterHandle::RegisterHandle(RegisterContext* context) : m_context(context) {
  m_context->inUse = true;
}

Context::RegisterHandle::~RegisterHandle() {
  m_context->inUse = false;
}

const Register& Context::RegisterHandle::reg() const {
  return m_context->reg;
}

std::string Context::RegisterHandle::asAsm(size_t size) const {
  return m_context->reg.asAsm(size);
}

Context::Context() : m_impl(std::make_unique<ContextImpl>()) {
  m_primitiveTypes = {{"i8", {1, 1}}, {"i16", {2, 2}}, {"i32", {4, 4}}, {"i64", {8, 8}}};
  m_registers = {
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

void Context::pushScope() {
  auto newContext = std::make_unique<ContextImpl>();
  newContext->stackOffset = m_impl->stackOffset;
  for (const auto& [size, offset] : m_impl->variables | std::views::values) {
    newContext->stackOffset += size;
  }
  newContext->parent = std::move(m_impl);
  m_impl = std::move(newContext);
}

void Context::pushFunction() {
  pushScope();
}

void Context::popScope() {
  m_impl = std::move(m_impl->parent);
}

void Context::popFunction() {
  popScope();
  m_localLabels = 0;
}

void Context::addVariable(const Parser::Nodes::VariableDeclaration* variable) {
  const Parser::Nodes::Node* typeIdent = variable->typeName()->child(Parser::Nodes::NodeType::Identifier);
  if (!typeIdent) {
    throw GenerationException("Missing identifier in type name");
  }

  const std::optional<Tokens::Token> identToken = typeIdent->token();
  if (!identToken || !identToken->value) {
    throw GenerationException("Invalid typename");
  }

  const std::optional<TypeContext> typeContext = type(*identToken->value);
  if (!typeContext) {
    throw GenerationException("Invalid type specified");
  }

  VariableContext varContext;
  varContext.offset = m_impl->stackOffset;
  for (const auto& [size, offset] : m_impl->variables | std::views::values) {
    varContext.offset += size;
  }
  // Align the variable
  varContext.offset =
      ((varContext.offset + typeContext->alignment - 1) / typeContext->alignment) * typeContext->alignment;

  varContext.size = typeContext->size;
  m_impl->variables.try_emplace(variable->name(), varContext);
}

std::optional<Context::VariableContext> Context::variable(const std::string& name) const {
  auto func = [name](const ContextImpl& context) -> std::optional<VariableContext> {
    if (context.variables.contains(name)) {
      return context.variables.at(name);
    }
    return std::nullopt;
  };

  return recurseContext<VariableContext>(*m_impl, func);
}

std::optional<Context::TypeContext> Context::type(const std::string& name) const {
  if (m_primitiveTypes.contains(name)) {
    return m_primitiveTypes.at(name);
  }

  auto func = [name](const ContextImpl& context) -> std::optional<TypeContext> {
    if (context.types.contains(name)) {
      return context.types.at(name);
    }
    return std::nullopt;
  };

  return recurseContext<TypeContext>(*m_impl, func);
}

size_t Context::nextLocalLabel() {
  return m_localLabels++;
}

std::unique_ptr<Context::RegisterHandle> Context::nextRegister() {
  for (auto& reg : m_registers) {
    if (!reg.inUse) {
      return std::make_unique<RegisterHandle>(&reg);
    }
  }
  throw GenerationException("Unable to get next register");
}

template <typename ReturnT>
std::optional<ReturnT> Context::recurseContext(
    const ContextImpl& context, std::function<std::optional<ReturnT>(const Context::ContextImpl&)> findFunc) const {
  if (auto ret = findFunc(context)) {
    return ret;
  }

  if (context.parent) return recurseContext(*(context.parent), findFunc);

  return {};
}
