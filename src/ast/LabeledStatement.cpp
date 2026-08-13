#include "LabeledStatement.h"
#include "AbstractSyntaxTreeVisitor.h"
namespace ast {
LabeledStatement::LabeledStatement(TerminalSymbol labelName, std::unique_ptr<AbstractSyntaxTreeNode> statement)
    : name{labelName}, statement{std::move(statement)} {}
void LabeledStatement::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
const std::string& LabeledStatement::getLabelName() const { return name.value; }
} // namespace ast
