#ifndef IFSTATEMENT_H_
#define IFSTATEMENT_H_

#include <memory>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Expression.h"
#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"

namespace ast {

class IfStatement: public AbstractSyntaxTreeNode {
public:
    IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~IfStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel);
    symbols::LabelEntry* getFalsyLabel(symbols::AnnotationStore& store) const;

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;

};

} // namespace ast

#endif // IFSTATEMENT_H_
