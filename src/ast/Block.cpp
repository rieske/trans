#include "Block.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"
#include "InitializedDeclarator.h"
#include "DirectDeclarator.h"

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

void Block::setSymbols(std::map<std::string, semantic_analyzer::ValueEntry> symbols) {
    this->symbols = symbols;
}

std::map<std::string, semantic_analyzer::ValueEntry> Block::getSymbols() const {
    return symbols;
}

void Block::setArguments(std::map<std::string, semantic_analyzer::ValueEntry> arguments) {
    this->arguments = arguments;
}

std::map<std::string, semantic_analyzer::ValueEntry> Block::getArguments() const {
    return arguments;
}

}
