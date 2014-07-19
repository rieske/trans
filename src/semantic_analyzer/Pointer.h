#ifndef _PTR_NODE_H_
#define _PTR_NODE_H_

#include <iostream>
#include <string>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class Pointer: public NonterminalNode {
public:
	Pointer(Pointer* pointer);
	Pointer();

	string getType() const;

	static const std::string ID;

private:
	string type;
};

}

#endif // _PTR_NODE_H_
