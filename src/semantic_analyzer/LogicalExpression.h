#ifndef LOGICALEXPRESSION_H_
#define LOGICALEXPRESSION_H_

#include <memory>

#include "Expression.h"

namespace semantic_analyzer {

class LogicalExpression: public Expression {
public:
    LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide,
            SymbolTable *st, unsigned ln);
    virtual ~LogicalExpression();

    std::unique_ptr<Expression> leftHandSide;
    std::unique_ptr<Expression> rightHandSide;

    void setExitLabel(SymbolEntry* exitLabel);
    SymbolEntry* getExitLabel() const;

private:
    SymbolEntry* exitLabel { nullptr };
};

} /* namespace semantic_analyzer */

#endif /* LOGICALEXPRESSION_H_ */
