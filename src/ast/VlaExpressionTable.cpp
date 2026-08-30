#include "VlaExpressionTable.h"

#include "Expression.h"

#include <stdexcept>

namespace ast {

void VlaExpressionTable::bind(const type::VlaBound* id, std::shared_ptr<Expression> expr) {
    if (!id || !expr) {
        throw std::logic_error { "VLA bind requires identity and expression" };
    }
    exprs_[id] = std::move(expr);
}

std::shared_ptr<Expression> VlaExpressionTable::lookup(const type::VlaBound* id) const {
    if (!id) {
        return {};
    }
    auto it = exprs_.find(id);
    if (it == exprs_.end()) {
        return {};
    }
    return it->second;
}

std::shared_ptr<Expression> VlaExpressionTable::require(const type::VlaBound* id) const {
    auto expr = lookup(id);
    if (!expr) {
        throw std::logic_error { "missing VLA bound expression" };
    }
    return expr;
}

} // namespace ast
