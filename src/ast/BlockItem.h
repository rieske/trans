#ifndef BLOCKITEM_H_
#define BLOCKITEM_H_

#include <memory>
#include <variant>

#include "Declaration.h"
#include "Expression.h"
#include "Statement.h"

namespace ast {

class AbstractSyntaxTreeVisitor;
class Block;

class BlockItem {
public:
    explicit BlockItem(std::unique_ptr<Declaration> declaration);
    explicit BlockItem(std::unique_ptr<Expression> expression);
    explicit BlockItem(std::unique_ptr<Statement> statement);

    const Declaration* asDeclaration() const;
    const Expression* asExpression() const;
    Expression* asExpression();

    std::unique_ptr<Expression> takeExpression();
    std::unique_ptr<Statement> takeStatement();
    std::unique_ptr<Block> takeBlock();

    void accept(AbstractSyntaxTreeVisitor& visitor) const;

private:
    std::variant<std::unique_ptr<Declaration>, std::unique_ptr<Expression>,
            std::unique_ptr<Statement>> item_;
};

} // namespace ast

#endif
