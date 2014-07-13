#ifndef _DECLS_NODE_H_
#define _DECLS_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "Declaration.h"

namespace semantic_analyzer {

class DeclarationList: public NonterminalNode {
public:
	DeclarationList(DeclarationList* declarationList, ParseTreeNode* comma, Declaration* declaration);
	DeclarationList(Declaration* declaration);

	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

	vector<Declaration *> getDecls() const;

	void outputDecls(ostringstream &oss) const;

	static const std::string ID;

private:
	vector<Declaration *> decls;
};

}

#endif // _DECLS_NODE_H_
