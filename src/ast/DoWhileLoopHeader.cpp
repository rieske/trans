#include "DoWhileLoopHeader.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

DoWhileLoopHeader::DoWhileLoopHeader(std::unique_ptr<Expression> clause) :
        clause { std::move(clause) } {
}

void DoWhileLoopHeader::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast
