#ifndef _SYNTAX_TREE_H_
#define _SYNTAX_TREE_H_

#include <iostream>

class SymbolTable;

namespace parser {

class ParseTreeNode;

class SyntaxTree {
public:
	virtual ~SyntaxTree();

	virtual SymbolTable* getSymbolTable() const;

	virtual void outputXml(std::ostream& stream) const = 0;
};

}

#endif // _SYNTAX_TREE_H_
