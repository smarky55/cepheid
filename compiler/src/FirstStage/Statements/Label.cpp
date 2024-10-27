#include "Label.h"

#include <FirstStage/Statements/StatementVisitor.h>

Cepheid::Eval::Label::Label(std::string_view name) : m_name(name) {
}

void Cepheid::Eval::Label::accept(StatementVisitor& visitor) {
  visitor.visit(*this);
}
