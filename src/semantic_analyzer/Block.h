#ifndef _BLOCK_NODE_H_
#define _BLOCK_NODE_H_

#include "Carrier.h"

namespace semantic_analyzer {

class Block: public Carrier {
public:
	Block(vector<ParseTreeNode *> &children);

	static const std::string ID;
};

}

#endif // _BLOCK_NODE_H_
