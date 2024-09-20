#pragma once
#include <vector>


namespace Cepheid::Eval {
class Statement;

class Function {

private:
  std::vector<Statement> m_statements;
};

}
