#ifndef IFELSESTATEMENT_H_
#define IFELSESTATEMENT_H_

#include <memory>

#include "../code_generator/LabelEntry.h"
#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Expression;

class IfElseStatement: public AbstractSyntaxTreeNode {
public:
    IfElseStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> truthyBody,
            std::unique_ptr<AbstractSyntaxTreeNode> falsyBody);
    virtual ~IfElseStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    code_generator::LabelEntry* getFalsyLabel() const;
    void setFalsyLabel(code_generator::LabelEntry falsyLabel);
    code_generator::LabelEntry* getExitLabel() const;
    void setExitLabel(code_generator::LabelEntry truthyLabel);

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> truthyBody;
    const std::unique_ptr<AbstractSyntaxTreeNode> falsyBody;

private:
    std::unique_ptr<code_generator::LabelEntry> exitLabel { nullptr };
    std::unique_ptr<code_generator::LabelEntry> falsyLabel { nullptr };
};

} /* namespace ast */

#endif /* IFELSESTATEMENT_H_ */
