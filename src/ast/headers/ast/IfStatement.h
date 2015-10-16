#ifndef IFSTATEMENT_H_
#define IFSTATEMENT_H_

#include <memory>

#include "semantic_analyzer/LabelEntry.h"
#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Expression;

class IfStatement: public AbstractSyntaxTreeNode {
public:
    IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~IfStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setFalsyLabel(semantic_analyzer::LabelEntry falsyLabel);
    semantic_analyzer::LabelEntry* getFalsyLabel() const;

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> falsyLabel { nullptr };
};

} /* namespace ast */

#endif /* IFSTATEMENT_H_ */
