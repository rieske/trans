#ifndef IFELSESTATEMENT_H_
#define IFELSESTATEMENT_H_

#include <memory>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Expression.h"
#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"

namespace ast {

class IfElseStatement: public AbstractSyntaxTreeNode {
public:
    IfElseStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> truthyBody,
            std::unique_ptr<AbstractSyntaxTreeNode> falsyBody);
    virtual ~IfElseStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    symbols::LabelEntry* getFalsyLabel(symbols::AnnotationStore& store) const;
    void setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel);
    symbols::LabelEntry* getExitLabel(symbols::AnnotationStore& store) const;
    void setExitLabel(symbols::AnnotationStore& store, symbols::LabelEntry exitLabel);

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> truthyBody;
    const std::unique_ptr<AbstractSyntaxTreeNode> falsyBody;

private:
};

} // namespace ast

#endif // IFELSESTATEMENT_H_
