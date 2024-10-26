#include "ReturnStatement.h"

#include <utility>

Cepheid::Eval::ReturnStatement::ReturnStatement(std::shared_ptr<Value> result) : m_result(std::move(result)) {
}
