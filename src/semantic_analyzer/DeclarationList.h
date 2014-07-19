#ifndef _DECLS_NODE_H_
#define _DECLS_NODE_H_

#include <string>
#include <vector>

#include "Declaration.h"

namespace semantic_analyzer {

class DeclarationList: public NonterminalNode {
public:
	DeclarationList(DeclarationList* declarationList, Declaration* declaration);
	DeclarationList(Declaration* declaration);

	vector<Declaration *> getDecls() const;

	static const std::string ID;

private:
	vector<Declaration *> decls;
};

}

#endif // _DECLS_NODE_H_
