#ifndef _CARRIER_NODE_H_
#define _CARRIER_NODE_H_

#include <string>
#include <vector>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class Carrier: public NonterminalNode {
public:
	Carrier(string label, vector<AbstractSyntaxTreeNode *> &children);
};

}

#endif // _CARRIER_NODE_H_
