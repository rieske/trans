#ifndef _A_EXPR_NODE_H_
#define _A_EXPR_NODE_H_

#include <memory>
#include <optional>
#include <string>

#include "DoubleOperandExpression.h"

namespace ast {

class AssignmentExpression: public DoubleOperandExpression {
public:
    AssignmentExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> assignmentOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    semantic_analyzer::ValueEntry* leftOperandLvalueSymbol() const;

    // Pointer += / -= integer: scale the integer by sizeof(pointee) (C 6.5.16.2).
    // scaleTemp holds (integer * scale) before Add/Sub into the pointer result.
    void setPointerArithmetic(int scale, std::string scaleTempName);
    int getPointerScale() const;
    const std::string* getScaleTempName() const;

private:
    int pointerScale { 0 };
    std::optional<std::string> scaleTempName;
};

} // namespace ast

#endif // _A_EXPR_NODE_H_
