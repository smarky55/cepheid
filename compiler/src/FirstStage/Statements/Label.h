#pragma once
#include <FirstStage/Statements/Statement.h>

#include <string>

namespace Cepheid::Eval {
class Label :public Statement {
public:
  explicit Label(std::string_view name);

private:
  std::string m_name;
};

}
