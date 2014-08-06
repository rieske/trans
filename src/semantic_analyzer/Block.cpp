#include "Block.h"

#include <algorithm>

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string Block::ID = "<block>";

Block::Block(std::unique_ptr<AbstractSyntaxTreeNode> subblock) {
	auto subblockCode = subblock->getCode();
	code.insert(code.end(), subblockCode.begin(), subblockCode.end());

	code.insert(code.begin(), new Quadruple(SCOPE, NULL, NULL, NULL));
	code.push_back(new Quadruple(ENDSCOPE, NULL, NULL, NULL));

	children.push_back(std::move(subblock));
}

Block::Block(std::unique_ptr<AbstractSyntaxTreeNode> firstSubblock, std::unique_ptr<AbstractSyntaxTreeNode> secondSubblock) {
	auto firstSubblockCode = firstSubblock->getCode();
	code.insert(code.end(), firstSubblockCode.begin(), firstSubblockCode.end());
	auto secondSubblockCode = secondSubblock->getCode();
	code.insert(code.end(), secondSubblockCode.begin(), secondSubblockCode.end());

	code.insert(code.begin(), new Quadruple(SCOPE, NULL, NULL, NULL));
	code.push_back(new Quadruple(ENDSCOPE, NULL, NULL, NULL));

	children.push_back(std::move(firstSubblock));
	children.push_back(std::move(secondSubblock));
}

Block::~Block() {
}

const std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>& Block::getChildren() const {
	return children;
}

void Block::accept(AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
