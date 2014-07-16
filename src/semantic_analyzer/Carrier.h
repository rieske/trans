#ifndef _CARRIER_NODE_H_
#define _CARRIER_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Carrier: public parser::NonterminalNode {
public:
	Carrier(string label, vector<ParseTreeNode *> &children);

	virtual std::ostringstream &asXml(std::ostringstream &oss, unsigned depth) const;
};

}

#endif // _CARRIER_NODE_H_
