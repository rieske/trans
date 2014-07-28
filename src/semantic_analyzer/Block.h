#ifndef _BLOCK_NODE_H_
#define _BLOCK_NODE_H_

#include <string>
#include <vector>

#include "Carrier.h"

namespace semantic_analyzer {

class Block: public Carrier {
public:
	Block(std::vector<AbstractSyntaxTreeNode*> children);

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string ID;
};

}

#endif // _BLOCK_NODE_H_
