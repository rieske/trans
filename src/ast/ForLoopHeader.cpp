#include "ForLoopHeader.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"

namespace ast {

ForLoopHeader::ForLoopHeader(std::unique_ptr<AbstractSyntaxTreeNode> initialization,
        std::unique_ptr<Expression> clause,
        std::unique_ptr<Expression> increment) :
        LoopHeader(std::move(increment)),
        initialization { std::move(initialization) },
        clause { std::move(clause) } {
}

ForLoopHeader::~ForLoopHeader() = default;

void ForLoopHeader::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

bool ForLoopHeader::isDeclarationInit() const {
    return initialization && dynamic_cast<const Declaration*>(initialization.get()) != nullptr;
}

} // namespace ast
