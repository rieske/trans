#include "GotoStatement.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

GotoStatement::GotoStatement(TerminalSymbol gotoKeyword, TerminalSymbol labelName) :
        gotoKeyword { gotoKeyword },
        label { labelName } {
}

void GotoStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void GotoStatement::setTarget(semantic_analyzer::LabelEntry target) {
    this->target = std::make_unique<semantic_analyzer::LabelEntry>(target);
}

semantic_analyzer::LabelEntry* GotoStatement::getTarget() const {
    return target.get();
}

const std::string& GotoStatement::getLabelName() const {
    return label.value;
}

} // namespace ast
