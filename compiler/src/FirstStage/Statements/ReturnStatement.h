#pragma once
#include <FirstStage/Statements/Statement.h>

namespace Cepheid::Eval {
class Value;

class ReturnStatement : public Statement {
 public:
  explicit ReturnStatement(std::shared_ptr<Value> result);

private:
  std::shared_ptr<Value> m_result;
};

}  // namespace Cepheid::Eval
