#ifndef _COMPARISON_EXPRESSION_H_
#define _COMPARISON_EXPRESSION_H_

#include <memory>
#include <string>

#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"
#include "BinaryOpExpression.h"

namespace ast {

class ComparisonExpression: public BinaryOpExpression {
public:
    ComparisonExpression(std::unique_ptr<Expression> leftHandSide, std::string lexeme,
            std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::Comparison; }
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    bool evaluateConstant(type::IntegerConstant& value) const override {
        return foldOperands(value, lexeme());
    }

    symbols::LabelEntry* getFalsyLabel(symbols::AnnotationStore& store) const;
    void setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel);
    symbols::LabelEntry* getTruthyLabel(symbols::AnnotationStore& store) const;
    void setTruthyLabel(symbols::AnnotationStore& store, symbols::LabelEntry truthyLabel);
};

} // namespace ast

#endif // _COMPARISON_EXPRESSION_H_
