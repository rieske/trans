#include "IfElseStatement.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace ast {

IfElseStatement::IfElseStatement(std::unique_ptr<Expression> testExpression,
        std::unique_ptr<AbstractSyntaxTreeNode> truthyBody, std::unique_ptr<AbstractSyntaxTreeNode> falsyBody) :
        testExpression { std::move(testExpression) },
        truthyBody { std::move(truthyBody) },
        falsyBody { std::move(falsyBody) }
{
}

IfElseStatement::~IfElseStatement() {
}

void IfElseStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

semantic_analyzer::LabelEntry* IfElseStatement::getFalsyLabel() const {
    return falsyLabel.get();
}

void IfElseStatement::setFalsyLabel(semantic_analyzer::LabelEntry falsyLabel) {
    this->falsyLabel = std::make_unique<semantic_analyzer::LabelEntry>(falsyLabel);
}

semantic_analyzer::LabelEntry* IfElseStatement::getExitLabel() const {
    return exitLabel.get();
}

void IfElseStatement::setExitLabel(semantic_analyzer::LabelEntry truthyLabel) {
    this->exitLabel = std::make_unique<semantic_analyzer::LabelEntry>(truthyLabel);
}

} /* namespace ast */

