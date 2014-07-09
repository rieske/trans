#ifndef _CARRIER_NODE_H_
#define _CARRIER_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../parser/nonterminal_node.h"

class CarrierNode: public NonterminalNode {
public:
	CarrierNode(string l, vector<ParseTreeNode *> &children);

	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;
};

#endif // _CARRIER_NODE_H_
