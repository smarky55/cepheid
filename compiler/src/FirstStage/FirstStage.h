#pragma once

namespace Cepheid::Parser::Nodes {
class Node;
}

namespace Cepheid::Eval {
class Module;

class FirstStage {
 public:
  static void add(const Parser::Nodes::Node& rootNode, Module& program);

 private:
  explicit FirstStage();
};
}  // namespace Cepheid::Eval
