#ifndef _U_EXPR_NODE_H_
#define _U_EXPR_NODE_H_

#include <memory>
#include <string>

#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"
#include "ast/UnaryOpExpression.h"

namespace ast {

class UnaryExpression: public UnaryOpExpression {
public:
    UnaryExpression(std::string lexeme, std::unique_ptr<Expression> castExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::Unary; }
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;

    bool isLval() const override;
    bool evaluateConstant(type::IntegerConstant& value) const override;

    void setTruthyLabel(symbols::AnnotationStore& store, symbols::LabelEntry truthyLabel);
    symbols::LabelEntry* getTruthyLabel(symbols::AnnotationStore& store) const;
    void setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel);
    symbols::LabelEntry* getFalsyLabel(symbols::AnnotationStore& store) const;

    void setSizeofValue(symbols::AnnotationStore& store, int bytes);
    const int* sizeofValue(const symbols::AnnotationStore& store) const;
};

} // namespace ast

#endif // _U_EXPR_NODE_H_
