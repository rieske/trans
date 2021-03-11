#ifndef _U_EXPR_NODE_H_
#define _U_EXPR_NODE_H_

#include <memory>

#include "semantic_analyzer/LabelEntry.h"
#include "ast/SingleOperandExpression.h"

namespace ast {

class UnaryExpression: public SingleOperandExpression {
public:
    UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setTruthyLabel(semantic_analyzer::LabelEntry truthyLabel);
    semantic_analyzer::LabelEntry* getTruthyLabel() const;
    void setFalsyLabel(semantic_analyzer::LabelEntry falsyLabel);
    semantic_analyzer::LabelEntry* getFalsyLabel() const;

    void setLvalueSymbol(semantic_analyzer::ValueEntry lvalueSymbol);
    semantic_analyzer::ValueEntry* getLvalueSymbol() const override;

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> truthyLabel { nullptr };
    std::unique_ptr<semantic_analyzer::LabelEntry> falsyLabel { nullptr };

    std::unique_ptr<semantic_analyzer::ValueEntry> lvalueSymbol { nullptr };
};

} // namespace ast

#endif // _U_EXPR_NODE_H_
