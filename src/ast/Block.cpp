#include "Block.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"

namespace ast {

const std::string Block::ID = "<compound_stat>";

Block::Block(std::vector<std::unique_ptr<Declaration>> declarations, std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> statements) :
        declarations { std::move(declarations) },
        statements { std::move(statements) }
{
}

void Block::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void Block::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    for (const auto& declaration : declarations) {
        declaration->accept(visitor);
    }
    for (const auto& statement : statements) {
        statement->accept(visitor);
    }
}

} // namespace ast

