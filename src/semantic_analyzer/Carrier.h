#ifndef _CARRIER_NODE_H_
#define _CARRIER_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Carrier: public NonterminalNode {
public:
	Carrier(string label, vector<ParseTreeNode *> &children);

	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;
};

}

#endif // _CARRIER_NODE_H_
