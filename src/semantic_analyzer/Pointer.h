#ifndef _PTR_NODE_H_
#define _PTR_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Pointer: public parser::NonterminalNode {
public:
	Pointer(Pointer* pointer);
	Pointer();

	virtual std::ostringstream &asXml(std::ostringstream &oss, unsigned depth) const;

	string getType() const;

	static const std::string ID;

private:
	string type;
};

}

#endif // _PTR_NODE_H_
