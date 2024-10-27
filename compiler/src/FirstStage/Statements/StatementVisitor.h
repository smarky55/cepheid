#pragma once

namespace Cepheid::Eval {
class BinaryOperation;
class JumpStatement;
class Label;
class ReturnStatement;
class UnaryOperation;

class StatementVisitor {
public:
  virtual ~StatementVisitor() = default;

  virtual void visit(const BinaryOperation&) = 0;
  virtual void visit(const UnaryOperation&) = 0;
  virtual void visit(const JumpStatement&) = 0;
  virtual void visit(const Label&) = 0;
  virtual void visit(const ReturnStatement&) = 0;
};

}
