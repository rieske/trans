#include "IfStatement.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

#include "Expression.h"

namespace semantic_analyzer {

IfStatement::IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body) :
        testExpression { std::move(testExpression) },
        body { std::move(body) } {
}

IfStatement::~IfStatement() {
}

void IfStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void IfStatement::setFalsyLabel(SymbolEntry* falsyLabel) {
    this->falsyLabel = falsyLabel;
}

SymbolEntry* IfStatement::getFalsyLabel() const {
    return falsyLabel;
}

} /* namespace semantic_analyzer */
