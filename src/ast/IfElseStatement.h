#ifndef IFELSESTATEMENT_H_
#define IFELSESTATEMENT_H_

#include <memory>

#include "symbols/LabelEntry.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Expression.h"

namespace ast {

class IfElseStatement: public AbstractSyntaxTreeNode {
public:
    IfElseStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> truthyBody,
            std::unique_ptr<AbstractSyntaxTreeNode> falsyBody);
    virtual ~IfElseStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    symbols::LabelEntry* getFalsyLabel() const;
    void setFalsyLabel(symbols::LabelEntry falsyLabel);
    symbols::LabelEntry* getExitLabel() const;
    void setExitLabel(symbols::LabelEntry truthyLabel);

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> truthyBody;
    const std::unique_ptr<AbstractSyntaxTreeNode> falsyBody;

};

} // namespace ast

#endif // IFELSESTATEMENT_H_
