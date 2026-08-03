#include "GotoStatement.h"
#include "AbstractSyntaxTreeVisitor.h"
namespace ast {
GotoStatement::GotoStatement(TerminalSymbol gotoKeyword, TerminalSymbol labelName)
    : gotoKeyword{std::move(gotoKeyword)}, label{std::move(labelName)} {}
void GotoStatement::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
const std::string& GotoStatement::getLabelName() const { return label.value; }
} // namespace ast
