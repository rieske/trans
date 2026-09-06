#ifndef FORINIT_H_
#define FORINIT_H_

#include <memory>
#include <variant>

#include "Declaration.h"
#include "Expression.h"

namespace ast {

class AbstractSyntaxTreeVisitor;

class ForInit {
public:
    ForInit() = default;
    explicit ForInit(std::unique_ptr<Declaration> declaration);
    explicit ForInit(std::unique_ptr<Expression> expression);

    const Declaration* asDeclaration() const;
    const Expression* asExpression() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) const;

private:
    std::variant<std::monostate, std::unique_ptr<Declaration>, std::unique_ptr<Expression>> item_;
};

} // namespace ast

#endif
