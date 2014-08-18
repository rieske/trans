#include "UnaryExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string UnaryExpression::ID { "<u_expr>" };

UnaryExpression::UnaryExpression(TerminalSymbol unaryOperator, std::unique_ptr<Expression> castExpression) :
        castExpression { std::move(castExpression) },
        unaryOperator { unaryOperator } {
    saveExpressionAttributes(*this->castExpression);
}

void UnaryExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void UnaryExpression::setTruthyLabel(SymbolEntry* truthyLabel) {
    this->truthyLabel = truthyLabel;
}

SymbolEntry* UnaryExpression::getTruthyLabel() const {
    return truthyLabel;
}

void UnaryExpression::setFalsyLabel(SymbolEntry* falsyLabel) {
    this->falsyLabel = falsyLabel;
}

SymbolEntry* UnaryExpression::getFalsyLabel() const {
    return falsyLabel;
}

}
