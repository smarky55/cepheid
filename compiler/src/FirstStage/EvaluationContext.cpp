#include "EvaluationContext.h"

#include <Parser/Node/VariableDeclaration.h>

#include <ranges>

using namespace Cepheid::Eval;

EvaluationContext::EvaluationContext() {
}

void EvaluationContext::push() {
  auto newContext = std::make_unique<ContextImpl>();
  newContext->parent = std::move(m_impl);
  m_impl = std::move(newContext);
}

void EvaluationContext::pop() {
  m_impl = std::move(m_impl->parent);
}

template <typename ReturnT>
std::optional<ReturnT> EvaluationContext::recurseContext(
    const ContextImpl& context, std::function<std::optional<ReturnT>(const EvaluationContext::ContextImpl&)> findFunc) const {
  if (auto ret = findFunc(context)) {
    return ret;
  }

  if (context.parent) return recurseContext(*(context.parent), findFunc);

  return {};
}
