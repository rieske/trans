#include "Declaration.h"
#include "Declarator.h"

#include <algorithm>
#include <stdexcept>

namespace ast {

const std::string Declaration::ID { "<decl>" };

Declaration::Declaration(DeclarationSpecifiers declarationSpecifiers, std::vector<std::unique_ptr<Declarator>> declarators) :
        declarationSpecifiers { declarationSpecifiers },
        declarators { std::move(declarators) }
{
}

Declaration::~Declaration() {
}

void Declaration::accept(AbstractSyntaxTreeVisitor& visitor) {
    throw new std::runtime_error { "Declaration::accept is not implemented yet" };
}

} /* namespace ast */
