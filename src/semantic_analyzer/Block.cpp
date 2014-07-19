#include "Block.h"

#include "../code_generator/quadruple.h"

namespace semantic_analyzer {

const std::string Block::ID = "<block>";

Block::Block(vector<AbstractSyntaxTreeNode *> &children) :
		Carrier(ID, children) {
	code.insert(code.begin(), new Quadruple(SCOPE, NULL, NULL, NULL));
	code.push_back(new Quadruple(ENDSCOPE, NULL, NULL, NULL));
}

}
