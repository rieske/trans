#include "DeclarationList.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"

namespace semantic_analyzer {

const std::string DeclarationList::ID { "<decls>" };

DeclarationList::DeclarationList(std::unique_ptr<Declaration> declaration) {
	declarations.push_back(std::move(declaration));
}

void DeclarationList::addDeclaration(std::unique_ptr<Declaration> declaration) {
	declarations.push_back(std::move(declaration));
}

const vector<std::unique_ptr<Declaration>>& DeclarationList::getDeclarations() const {
	return declarations;
}

void DeclarationList::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
