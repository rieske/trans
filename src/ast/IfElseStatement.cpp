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

code_generator::LabelEntry* IfElseStatement::getFalsyLabel() const {
    return falsyLabel.get();
}

void IfElseStatement::setFalsyLabel(code_generator::LabelEntry falsyLabel) {
    this->falsyLabel = std::unique_ptr<code_generator::LabelEntry> { new code_generator::LabelEntry { falsyLabel } };
}

code_generator::LabelEntry* IfElseStatement::getExitLabel() const {
    return exitLabel.get();
}

void IfElseStatement::setExitLabel(code_generator::LabelEntry truthyLabel) {
    this->exitLabel = std::unique_ptr<code_generator::LabelEntry> { new code_generator::LabelEntry { truthyLabel } };
}

} /* namespace ast */

