#include "DereferencedDeclarator.h"

#include <algorithm>

#include "../code_generator/ValueEntry.h"
#include "../translation_unit/Context.h"

namespace ast {

DereferencedDeclarator::DereferencedDeclarator(std::unique_ptr<Declarator> declaration) :
        declaration { std::move(declaration) }
{
}

DereferencedDeclarator::~DereferencedDeclarator() {
}

void DereferencedDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    declaration->accept(visitor);
}

std::string DereferencedDeclarator::getName() const {
    return declaration->getName();
}

int DereferencedDeclarator::getDereferenceCount() const {
    return declaration->getDereferenceCount() + 1;
}

translation_unit::Context DereferencedDeclarator::getContext() const {
    return declaration->getContext();
}

void DereferencedDeclarator::setHolder(code_generator::ValueEntry holder) {
    declaration->setHolder(holder);
}

code_generator::ValueEntry* DereferencedDeclarator::getHolder() const {
    return declaration->getHolder();
}

} /* namespace ast */
