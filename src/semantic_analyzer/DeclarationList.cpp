#include "DeclarationList.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string DeclarationList::ID { "<decls>" };

DeclarationList::DeclarationList(Declaration* declaration) {
	declarations.push_back(declaration);
}

void DeclarationList::addDeclaration(Declaration* declaration) {
	declarations.push_back(declaration);
}

vector<Declaration *> DeclarationList::getDeclarations() const {
	return declarations;
}

void DeclarationList::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
