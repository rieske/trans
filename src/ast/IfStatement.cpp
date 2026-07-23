#include "IfStatement.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
IfStatement::IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body)
    : testExpression{std::move(testExpression)}, body{std::move(body)} {}
IfStatement::~IfStatement() = default;
void IfStatement::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void IfStatement::setFalsyLabel(symbols::LabelEntry falsyLabel) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Falsy, std::move(falsyLabel));
}
symbols::LabelEntry* IfStatement::getFalsyLabel() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Falsy);
}
} // namespace ast
