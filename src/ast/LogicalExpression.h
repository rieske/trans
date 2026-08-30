#ifndef LOGICALEXPRESSION_H_
#define LOGICALEXPRESSION_H_

#include <memory>

#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"
#include "ast/DoubleOperandExpression.h"

namespace ast {

class LogicalExpression: public DoubleOperandExpression {
public:
    virtual ~LogicalExpression();

    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;

    void setExitLabel(symbols::AnnotationStore& store, symbols::LabelEntry exitLabel);
    symbols::LabelEntry* getExitLabel(symbols::AnnotationStore& store) const;

protected:
    LogicalExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide);

};

} // namespace ast

#endif // LOGICALEXPRESSION_H_
