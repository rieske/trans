#include "Block.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string Block::ID = "<block>";

Block::Block(std::unique_ptr<AbstractSyntaxTreeNode> subblock) {
    children.push_back(std::move(subblock));
}

Block::Block(std::unique_ptr<AbstractSyntaxTreeNode> firstSubblock,
        std::unique_ptr<AbstractSyntaxTreeNode> secondSubblock) {
    children.push_back(std::move(firstSubblock));
    children.push_back(std::move(secondSubblock));
}

Block::~Block() {
}

const std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>& Block::getChildren() const {
    return children;
}

void Block::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
