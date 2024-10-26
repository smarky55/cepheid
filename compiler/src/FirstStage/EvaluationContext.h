#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace Cepheid::Eval {
class EvaluationContext {
 public:
  EvaluationContext();
  EvaluationContext(const EvaluationContext& other) = delete;
  EvaluationContext(EvaluationContext&& other) noexcept = delete;
  EvaluationContext& operator=(const EvaluationContext& other) = delete;
  EvaluationContext& operator=(EvaluationContext&& other) noexcept = delete;
  ~EvaluationContext() = default;

  void push();

  void pop();

 private:
  struct ContextImpl {
    std::unique_ptr<ContextImpl> parent;
  };

  template <typename ReturnT>
  [[nodiscard]] std::optional<ReturnT> recurseContext(
      const ContextImpl& context, std::function<std::optional<ReturnT>(const ContextImpl&)>) const;

  std::unique_ptr<ContextImpl> m_impl;
};

}  // namespace Cepheid::Eval
