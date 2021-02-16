#ifndef IFELSESTATEMENT_H_
#define IFELSESTATEMENT_H_

#include <memory>

#include "semantic_analyzer/LabelEntry.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Expression.h"

namespace ast {

class IfElseStatement: public AbstractSyntaxTreeNode {
public:
    IfElseStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> truthyBody,
            std::unique_ptr<AbstractSyntaxTreeNode> falsyBody);
    virtual ~IfElseStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    semantic_analyzer::LabelEntry* getFalsyLabel() const;
    void setFalsyLabel(semantic_analyzer::LabelEntry falsyLabel);
    semantic_analyzer::LabelEntry* getExitLabel() const;
    void setExitLabel(semantic_analyzer::LabelEntry truthyLabel);

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> truthyBody;
    const std::unique_ptr<AbstractSyntaxTreeNode> falsyBody;

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> exitLabel { nullptr };
    std::unique_ptr<semantic_analyzer::LabelEntry> falsyLabel { nullptr };
};

} // namespace ast

#endif // IFELSESTATEMENT_H_
