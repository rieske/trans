#include "LoopStatement.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

LoopStatement::LoopStatement(std::unique_ptr<LoopHeader> header, std::unique_ptr<Statement> body) :
        header { std::move(header) },
        body { std::move(body) } {
}

void LoopStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

