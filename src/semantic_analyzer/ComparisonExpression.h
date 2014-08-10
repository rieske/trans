#ifndef _ML_EXPR_NODE_H_
#define _ML_EXPR_NODE_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class TerminalSymbol;

class ComparisonExpression: public Expression {
public:
    ComparisonExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol comparisonOperator,
            std::unique_ptr<Expression> rightHandSide, SymbolTable *st);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    SymbolEntry* getFalsyLabel() const;
    void setFalsyLabel(SymbolEntry* falsyLabel);
    SymbolEntry* getTruthyLabel() const;
    void setTruthyLabel(SymbolEntry* truthyLabel);

    static const std::string DIFFERENCE;
    static const std::string EQUALITY;

    const std::unique_ptr<Expression> leftHandSide;
    const TerminalSymbol comparisonOperator;
    const std::unique_ptr<Expression> rightHandSide;

private:
    SymbolEntry *truthyLabel { nullptr };
    SymbolEntry *falsyLabel { nullptr };
};

}

#endif // _ML_EXPR_NODE_H_
