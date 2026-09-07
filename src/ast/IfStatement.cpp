#include "IfStatement.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

IfStatement::IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<Statement> body,
        std::unique_ptr<Statement> elseBody) :
        testExpression { std::move(testExpression) },
        body { std::move(body) },
        elseBody { std::move(elseBody) } {
}

void IfStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void IfStatement::setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel) {
    store.setLabel(this, symbols::LabelSlot::Falsy, std::move(falsyLabel));
}

symbols::LabelEntry* IfStatement::getFalsyLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Falsy);
}

void IfStatement::setExitLabel(symbols::AnnotationStore& store, symbols::LabelEntry exitLabel) {
    store.setLabel(this, symbols::LabelSlot::Exit, std::move(exitLabel));
}

symbols::LabelEntry* IfStatement::getExitLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Exit);
}

} // namespace ast
