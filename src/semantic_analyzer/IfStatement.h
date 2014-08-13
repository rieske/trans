#ifndef IFSTATEMENT_H_
#define IFSTATEMENT_H_

#include <memory>

#include "AbstractSyntaxTreeNode.h"
#include "../code_generator/symbol_entry.h"


namespace semantic_analyzer {

class Expression;

class IfStatement: public AbstractSyntaxTreeNode {
public:
    IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~IfStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setFalsyLabel(SymbolEntry* falsyLabel);
    SymbolEntry* getFalsyLabel() const;

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;

private:
    SymbolEntry* falsyLabel { nullptr };
};

} /* namespace semantic_analyzer */

#endif /* IFSTATEMENT_H_ */
