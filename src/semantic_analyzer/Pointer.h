#ifndef _PTR_NODE_H_
#define _PTR_NODE_H_

#include <string>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class Pointer: public NonterminalNode {
public:
	Pointer();

	void dereference();
	string getType() const;

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string ID;

private:
	string type;
};

}

#endif // _PTR_NODE_H_
