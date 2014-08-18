#ifndef _U_EXPR_NODE_H_
#define _U_EXPR_NODE_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class TerminalSymbol;

class UnaryExpression: public Expression {
public:
    UnaryExpression(TerminalSymbol unaryOperator, std::unique_ptr<Expression> castExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setTruthyLabel(SymbolEntry* truthyLabel);
    SymbolEntry* getTruthyLabel() const;
    void setFalsyLabel(SymbolEntry* falsyLabel);
    SymbolEntry* getFalsyLabel() const;

    static const std::string ID;

    TerminalSymbol unaryOperator;
    const std::unique_ptr<Expression> castExpression;

private:
    SymbolEntry* truthyLabel { nullptr };
    SymbolEntry* falsyLabel { nullptr };
};

}

#endif // _U_EXPR_NODE_H_
