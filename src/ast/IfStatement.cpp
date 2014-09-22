#include "IfStatement.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace ast {

IfStatement::IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body) :
        testExpression { std::move(testExpression) }, body { std::move(body) } {
}

IfStatement::~IfStatement() {
}

void IfStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void IfStatement::setFalsyLabel(code_generator::LabelEntry* falsyLabel) {
    this->falsyLabel = falsyLabel;
}

code_generator::LabelEntry* IfStatement::getFalsyLabel() const {
    return falsyLabel;
}

} /* namespace ast */
