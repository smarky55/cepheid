#pragma once
#include <memory>
#include <vector>
#include <vector>

namespace Cepheid::Eval {
class StatementVisitor;

class Statement {
 public:
  virtual ~Statement() = default;

  virtual void accept(StatementVisitor& visitor) = 0;
};

}  // namespace Cepheid::Eval
