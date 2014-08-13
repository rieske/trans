#include "IfElseStatement.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

#include "Expression.h"

namespace semantic_analyzer {

IfElseStatement::IfElseStatement(std::unique_ptr<Expression> testExpression,
        std::unique_ptr<AbstractSyntaxTreeNode> truthyBody, std::unique_ptr<AbstractSyntaxTreeNode> falsyBody) :
        testExpression { std::move(testExpression) },
        truthyBody { std::move(truthyBody) },
        falsyBody { std::move(falsyBody) } {
}

IfElseStatement::~IfElseStatement() {
}

void IfElseStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

SymbolEntry* IfElseStatement::getFalsyLabel() const {
    return falsyLabel;
}

void IfElseStatement::setFalsyLabel(SymbolEntry* falsyLabel) {
    this->falsyLabel = falsyLabel;
}

SymbolEntry* IfElseStatement::getExitLabel() const {
    return exitLabel;
}

void IfElseStatement::setExitLabel(SymbolEntry* truthyLabel) {
    this->exitLabel = truthyLabel;
}

} /* namespace semantic_analyzer */

