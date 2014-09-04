#include "DeclarationList.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"

namespace ast {

const std::string DeclarationList::ID { "<decls>" };

DeclarationList::DeclarationList(std::unique_ptr<Declaration> declaration) {
	declarations.push_back(std::move(declaration));
}

void DeclarationList::addDeclaration(std::unique_ptr<Declaration> declaration) {
	declarations.push_back(std::move(declaration));
}

const std::vector<std::unique_ptr<Declaration>>& DeclarationList::getDeclarations() const {
	return declarations;
}

void DeclarationList::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

}
