#ifndef _DECL_NODE_H_
#define _DECL_NODE_H_

#include <iostream>
#include <string>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class DirectDeclaration;
class Pointer;

class Declaration: public NonterminalNode {
public:
	Declaration(Pointer* pointer, DirectDeclaration* directDeclaration);
	Declaration(DirectDeclaration* directDeclaration);

	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

	void output_attr(ostringstream &oss, unsigned nr) const;

	string getName() const;
	string getType() const;

	static const std::string ID;

private:
	string name;
	string type;
};

}

#endif // _DECL_NODE_H_
