#ifndef IFSTATEMENT_H_
#define IFSTATEMENT_H_

#include <memory>

#include "AbstractSyntaxTreeNode.h"

namespace code_generator {
class LabelEntry;
} /* namespace code_generator */

namespace ast {

class Expression;

class IfStatement: public AbstractSyntaxTreeNode {
public:
    IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~IfStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setFalsyLabel(code_generator::LabelEntry* falsyLabel);
    code_generator::LabelEntry* getFalsyLabel() const;

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;

private:
    code_generator::LabelEntry* falsyLabel { nullptr };
};

} /* namespace ast */

#endif /* IFSTATEMENT_H_ */
