#include "IOStatement.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

#include "Expression.h"

namespace ast {

const std::string IOStatement::ID { "<io_stmt>" };

IOStatement::IOStatement(TerminalSymbol ioKeyword, std::unique_ptr<Expression> expression) :
        ioKeyword { ioKeyword },
        expression { std::move(expression) } {
}

void IOStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
