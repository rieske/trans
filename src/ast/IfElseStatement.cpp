#include "IfElseStatement.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
IfElseStatement::IfElseStatement(std::unique_ptr<Expression> testExpression,
        std::unique_ptr<AbstractSyntaxTreeNode> truthyBody, std::unique_ptr<AbstractSyntaxTreeNode> falsyBody)
    : testExpression{std::move(testExpression)}, truthyBody{std::move(truthyBody)}, falsyBody{std::move(falsyBody)} {}
IfElseStatement::~IfElseStatement() = default;
void IfElseStatement::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void IfElseStatement::setFalsyLabel(symbols::LabelEntry falsyLabel) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Falsy, std::move(falsyLabel));
}
symbols::LabelEntry* IfElseStatement::getFalsyLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Falsy);
}
void IfElseStatement::setExitLabel(symbols::LabelEntry exitLabel) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Exit, std::move(exitLabel));
}
symbols::LabelEntry* IfElseStatement::getExitLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Exit);
}
} // namespace ast
