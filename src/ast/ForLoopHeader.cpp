#include "ForLoopHeader.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ForLoopHeader::ForLoopHeader(std::unique_ptr<AbstractSyntaxTreeNode> initialization,
        std::unique_ptr<Expression> clause,
        std::unique_ptr<Expression> increment,
        bool declarationScoped) :
        LoopHeader(std::move(increment)),
        initialization { std::move(initialization) },
        clause { std::move(clause) },
        declarationScoped { declarationScoped } {
}

ForLoopHeader::~ForLoopHeader() = default;

void ForLoopHeader::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace ast
