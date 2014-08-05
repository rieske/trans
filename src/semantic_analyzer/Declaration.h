#ifndef _DECL_NODE_H_
#define _DECL_NODE_H_

#include <string>

#include "NonterminalNode.h"
#include "TypeSpecifier.h"

namespace semantic_analyzer {

class DirectDeclaration;
class Pointer;

class Declaration: public NonterminalNode {
public:
	virtual ~Declaration();

	string getName() const;
	std::string getType() const;

	void dereference(int dereferenceCount);

	static const std::string ID;

protected:
	Declaration();
	Declaration(SymbolTable *st, unsigned ln);

	std::string name;
	std::string type;
	int dereferenceCount {0};
};

}

#endif // _DECL_NODE_H_
