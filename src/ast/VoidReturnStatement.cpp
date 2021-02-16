#include "VoidReturnStatement.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

void VoidReturnStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} // namespace ast
