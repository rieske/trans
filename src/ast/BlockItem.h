#ifndef BLOCKITEM_H_
#define BLOCKITEM_H_

#include <memory>
#include <variant>

#include "Declaration.h"
#include "Expression.h"

namespace ast {

class AbstractSyntaxTreeVisitor;

class BlockItem {
public:
    explicit BlockItem(std::unique_ptr<Declaration> declaration);
    explicit BlockItem(std::unique_ptr<Expression> expression);
    explicit BlockItem(std::unique_ptr<AbstractSyntaxTreeNode> statement);

    static BlockItem fromNode(std::unique_ptr<AbstractSyntaxTreeNode> node);

    const Declaration* asDeclaration() const;
    Declaration* asDeclaration();
    const Expression* asExpression() const;
    Expression* asExpression();

    void accept(AbstractSyntaxTreeVisitor& visitor) const;

private:
    std::variant<std::unique_ptr<Declaration>, std::unique_ptr<Expression>,
            std::unique_ptr<AbstractSyntaxTreeNode>> item_;
};

} // namespace ast

#endif
