#include "Declaration.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "InitializedDeclarator.h"

namespace ast {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration(DeclarationSpecifiers declarationSpecifiers, std::vector<std::unique_ptr<InitializedDeclarator>> declarators) :
        declarationSpecifiers { declarationSpecifiers },
        declarators { std::move(declarators) }
{
}

void Declaration::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace ast */
