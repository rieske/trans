#ifndef _PTR_NODE_H_
#define _PTR_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Pointer: public NonterminalNode {
public:
	Pointer(Pointer* pointer, ParseTreeNode* dereferenceOperator);
	Pointer(ParseTreeNode* dereferenceOperator);

	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

	string getType() const;

	static const std::string ID;

private:
	string type;
};

}

#endif // _PTR_NODE_H_
