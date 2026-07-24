#ifndef IFSTATEMENT_H_
#define IFSTATEMENT_H_

#include <memory>

#include "symbols/LabelEntry.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Expression.h"

namespace ast {

class IfStatement: public AbstractSyntaxTreeNode {
public:
    IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~IfStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setFalsyLabel(symbols::LabelEntry falsyLabel);
    symbols::LabelEntry* getFalsyLabel() const;

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;

private:
};

} // namespace ast

#endif // IFSTATEMENT_H_
