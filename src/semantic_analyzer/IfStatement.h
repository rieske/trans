#ifndef IFSTATEMENT_H_
#define IFSTATEMENT_H_

#include <memory>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class Expression;

class IfStatement: public NonterminalNode {
public:
    IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body,
            SymbolTable *st);
    virtual ~IfStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setFalsyLabel(SymbolEntry* falsyLabel);
    const SymbolEntry* getFalsyLabel() const;

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;

private:
    SymbolEntry* falsyLabel { nullptr };
};

} /* namespace semantic_analyzer */

#endif /* IFSTATEMENT_H_ */
