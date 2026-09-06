#include "NullStatement.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

void NullStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast
