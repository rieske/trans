#include "JumpStatement.h"
#include "AbstractSyntaxTreeVisitor.h"
namespace ast {
JumpStatement::JumpStatement(TerminalSymbol jumpKeyword) : jumpKeyword{jumpKeyword} {}
void JumpStatement::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
bool JumpStatement::isBreak() const { return jumpKeyword.type == "break"; }
} // namespace ast
