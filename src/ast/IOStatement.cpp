#include "IOStatement.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string IOStatement::ID { "<io_stat>" };

IOStatement::IOStatement(TerminalSymbol ioKeyword, std::unique_ptr<Expression> expression) :
        ioKeyword { ioKeyword },
        expression { std::move(expression) } {
}

void IOStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast

