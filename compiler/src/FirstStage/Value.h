#pragma once

namespace Cepheid::Eval {
class Type;

class Value {
 public:
  explicit Value(const Type* type);
  virtual ~Value() = default;

  const Type* type() const;

 private:
  const Type* m_type;
};

}  // namespace Cepheid::Eval
