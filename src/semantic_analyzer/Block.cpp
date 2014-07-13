#include "Block.h"

namespace semantic_analyzer {

const std::string Block::ID = "<block>";

Block::Block(vector<ParseTreeNode *> &children) :
		Carrier(ID, children) {
	code.insert(code.begin(), new Quadruple(SCOPE, NULL, NULL, NULL));
	code.push_back(new Quadruple(ENDSCOPE, NULL, NULL, NULL));
}

}
