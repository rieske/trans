#ifndef IFELSESTATEMENT_H_
#define IFELSESTATEMENT_H_

#include <memory>

#include "AbstractSyntaxTreeNode.h"
#include "../code_generator/symbol_entry.h"


namespace ast {

class Expression;

class IfElseStatement: public AbstractSyntaxTreeNode {
public:
    IfElseStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> truthyBody,
            std::unique_ptr<AbstractSyntaxTreeNode> falsyBody);
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

} /* namespace ast */

#endif /* IFELSESTATEMENT_H_ */
