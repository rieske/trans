#ifndef AST_VLAEXPRESSIONTABLE_H_
#define AST_VLAEXPRESSIONTABLE_H_

#include <memory>
#include <unordered_map>

#include "types/Type.h"

namespace ast {

class Expression;

class VlaExpressionTable {
public:
    void bind(const type::VlaBound* id, std::shared_ptr<Expression> expr);
    std::shared_ptr<Expression> lookup(const type::VlaBound* id) const;
    std::shared_ptr<Expression> require(const type::VlaBound* id) const;

private:
    std::unordered_map<const type::VlaBound*, std::shared_ptr<Expression>> exprs_;
};

} // namespace ast

#endif
