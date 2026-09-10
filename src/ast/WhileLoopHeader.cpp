#include "WhileLoopHeader.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

WhileLoopHeader::WhileLoopHeader(std::unique_ptr<Expression> clause) :
        clause { std::move(clause) } {
}

void WhileLoopHeader::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast
