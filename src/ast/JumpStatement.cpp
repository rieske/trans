#include "JumpStatement.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
JumpStatement::JumpStatement(TerminalSymbol jumpKeyword) : jumpKeyword{jumpKeyword} {}
void JumpStatement::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void JumpStatement::setTarget(symbols::LabelEntry target) {
    symbols::AnnotationStore::current().setLabel(this, symbols::LabelSlot::Target, std::move(target));
}
symbols::LabelEntry* JumpStatement::getTarget() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::Target);
}
bool JumpStatement::isBreak() const { return jumpKeyword.type == "break"; }
} // namespace ast
