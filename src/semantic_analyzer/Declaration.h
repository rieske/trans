#ifndef _DECL_NODE_H_
#define _DECL_NODE_H_

#include <iostream>
#include <string>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class DirectDeclaration;
class Pointer;

class Declaration: public NonterminalNode {
public:
	virtual ~Declaration();

	string getName() const;
	string getType() const;

	void dereference(std::string pointerType);

	static const std::string ID;

protected:
	Declaration();
	Declaration(SymbolTable *st, unsigned ln);

	string name;
	string type;
};

}

#endif // _DECL_NODE_H_
