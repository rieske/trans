#include "ast/LoopStatement.h"

#include <algorithm>

#include "ast/AbstractSyntaxTreeVisitor.h"

#include "ast/LoopHeader.h"

namespace ast {

LoopStatement::LoopStatement(std::unique_ptr<LoopHeader> header, std::unique_ptr<AbstractSyntaxTreeNode> body) :
        header { std::move(header) },
        body { std::move(body) } {
}

LoopStatement::~LoopStatement() {
}

void LoopStatement::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace ast */