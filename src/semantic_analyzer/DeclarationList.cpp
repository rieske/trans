#include "DeclarationList.h"

namespace semantic_analyzer {

const std::string DeclarationList::ID { "<decls>" };

DeclarationList::DeclarationList(DeclarationList* declarationList, Declaration* declaration) :
		NonterminalNode(ID, { declarationList, declaration }) {
	decls = declarationList->getDecls();
	decls.push_back(declaration);
}

DeclarationList::DeclarationList(Declaration* declaration) :
		NonterminalNode(ID, { declaration }) {
	decls.push_back(declaration);
}

vector<Declaration *> DeclarationList::getDecls() const {
	return decls;
}

}
