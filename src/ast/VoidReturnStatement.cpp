#include "VoidReturnStatement.h"

#include <stddef.h>
#include <algorithm>
#include <vector>

#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace ast {

void VoidReturnStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} /* namespace ast */
