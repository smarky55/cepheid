#pragma once

#include <FirstStage/Statements/Statement.h>

#include <string>

namespace Cepheid::Eval {
class Value;

class JumpStatement : public Statement {
 public:
  enum class JumpIf { True, False, Always };

  JumpStatement(std::string_view destination, JumpIf jumpIf, std::shared_ptr<Value> comparisonValue = nullptr);

 private:
  std::string m_destination;
  std::shared_ptr<Value> m_comparison;
  JumpIf m_jumpIf;
};

}  // namespace Cepheid::Eval
