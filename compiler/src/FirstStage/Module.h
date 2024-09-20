#pragma once

#include <string_view>
#include <vector>

namespace Cepheid::Eval {
class Function;

class Module {
 public:
  explicit Module(std::string_view name);

 private:
  std::string m_name;
  std::vector<Module> m_modules;
  std::vector<Function> m_functions;
};

}  // namespace Cepheid::Eval
