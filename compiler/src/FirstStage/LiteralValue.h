#pragma once
#include <FirstStage/Value.h>

namespace Cepheid::Eval {
template <typename LiteralT>
class LiteralValue : public Value {
 public:
  LiteralValue(const Type* type, LiteralT value);

 private:
  LiteralT m_value;
};

template <typename LiteralT>
LiteralValue<LiteralT>::LiteralValue(const Type* type, LiteralT value) : Value(type), m_value(value) {
}
}  // namespace Cepheid::Eval
