#pragma once
#include <string>
#include <FirstStage/Value.h>

namespace Cepheid::Eval {
class VariableValue : public Value {
public:
  VariableValue(const Type* type, const std::string& name);

private:
  std::string m_name;
};

}
