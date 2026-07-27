#include "IfElseStatement.h"

#include "AbstractSyntaxTreeVisitor.h"

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

symbols::LabelEntry* IfElseStatement::getFalsyLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Falsy);
}

void IfElseStatement::setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel) {
    store.setLabel(this, symbols::LabelSlot::Falsy, std::move(falsyLabel));
}

symbols::LabelEntry* IfElseStatement::getExitLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Exit);
}

void IfElseStatement::setExitLabel(symbols::AnnotationStore& store, symbols::LabelEntry exitLabel) {
    store.setLabel(this, symbols::LabelSlot::Exit, std::move(exitLabel));
}

} // namespace ast
