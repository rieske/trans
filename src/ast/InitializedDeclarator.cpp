#include "InitializedDeclarator.h"

#include <algorithm>
#include <stdexcept>

#include "../code_generator/ValueEntry.h"
#include "../translation_unit/Context.h"
#include "Declarator.h"
#include "Expression.h"

namespace ast {

const std::string InitializedDeclarator::ID { "<init_declarator>" };

InitializedDeclarator::InitializedDeclarator(std::unique_ptr<Declarator> declarator, std::unique_ptr<Expression> initializer) :
        declarator { std::move(declarator) },
        initializer { std::move(initializer) }
{
}

void InitializedDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    throw std::runtime_error { "InitializedDeclarator::accept is not implemented yet" };
    //visitor.visit(*this);
}

std::string ast::InitializedDeclarator::getName() const {
    return declarator->getName();
}

translation_unit::Context ast::InitializedDeclarator::getContext() const {
    return declarator->getContext();
}

} /* namespace ast */

