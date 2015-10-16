#include "ast/IfStatement.h"

#include <algorithm>

#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/Expression.h"

namespace ast {

IfStatement::IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body) :
        testExpression { std::move(testExpression) },
        body { std::move(body) }
{
}

IfStatement::~IfStatement() {
}

void IfStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void IfStatement::setFalsyLabel(semantic_analyzer::LabelEntry falsyLabel) {
    this->falsyLabel = std::make_unique<semantic_analyzer::LabelEntry>(falsyLabel);
}

semantic_analyzer::LabelEntry* IfStatement::getFalsyLabel() const {
    return falsyLabel.get();
}

} /* namespace ast */
