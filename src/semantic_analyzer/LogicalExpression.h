#ifndef LOGICALEXPRESSION_H_
#define LOGICALEXPRESSION_H_

#include <memory>

#include "Expression.h"
#include "../code_generator/symbol_entry.h"

namespace semantic_analyzer {

class LogicalExpression: public Expression {
public:
    virtual ~LogicalExpression();

    std::unique_ptr<Expression> leftHandSide;
    std::unique_ptr<Expression> rightHandSide;

    void setExitLabel(SymbolEntry* exitLabel);
    SymbolEntry* getExitLabel() const;

protected:
    LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide);

private:
    SymbolEntry* exitLabel { nullptr };
};

} /* namespace semantic_analyzer */

#endif /* LOGICALEXPRESSION_H_ */
