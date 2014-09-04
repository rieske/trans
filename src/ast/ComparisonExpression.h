#ifndef _ML_EXPR_NODE_H_
#define _ML_EXPR_NODE_H_

#include <memory>
#include <string>

#include "DoubleOperandExpression.h"
#include "TerminalSymbol.h"

namespace ast {

class ComparisonExpression: public DoubleOperandExpression {
public:
    ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> comparisonOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    SymbolEntry* getFalsyLabel() const;
    void setFalsyLabel(SymbolEntry* falsyLabel);
    SymbolEntry* getTruthyLabel() const;
    void setTruthyLabel(SymbolEntry* truthyLabel);

    static const std::string DIFFERENCE;
    static const std::string EQUALITY;

private:
    SymbolEntry *truthyLabel { nullptr };
    SymbolEntry *falsyLabel { nullptr };
};

}

#endif // _ML_EXPR_NODE_H_
