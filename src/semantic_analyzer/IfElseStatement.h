#ifndef IFELSESTATEMENT_H_
#define IFELSESTATEMENT_H_

#include <memory>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class Expression;

class IfElseStatement: public NonterminalNode {
public:
    IfElseStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> truthyBody,
            std::unique_ptr<AbstractSyntaxTreeNode> falsyBody, SymbolTable *st);
    virtual ~IfElseStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    SymbolEntry* getFalsyLabel() const;
    void setFalsyLabel(SymbolEntry* falsyLabel);
    SymbolEntry* getExitLabel() const;
    void setExitLabel(SymbolEntry* truthyLabel);

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> truthyBody;
    const std::unique_ptr<AbstractSyntaxTreeNode> falsyBody;

private:
    SymbolEntry* exitLabel { nullptr };
    SymbolEntry* falsyLabel { nullptr };
};

} /* namespace semantic_analyzer */

#endif /* IFELSESTATEMENT_H_ */
