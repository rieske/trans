#ifndef _U_EXPR_NODE_H_
#define _U_EXPR_NODE_H_

#include <memory>
#include <string>

#include "SingleOperandExpression.h"
#include "TerminalSymbol.h"

namespace ast {

class TerminalSymbol;

class UnaryExpression: public SingleOperandExpression {
public:
    UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setTruthyLabel(SymbolEntry* truthyLabel);
    SymbolEntry* getTruthyLabel() const;
    void setFalsyLabel(SymbolEntry* falsyLabel);
    SymbolEntry* getFalsyLabel() const;

    static const std::string ID;

private:
    SymbolEntry* truthyLabel { nullptr };
    SymbolEntry* falsyLabel { nullptr };
};

}

#endif // _U_EXPR_NODE_H_
