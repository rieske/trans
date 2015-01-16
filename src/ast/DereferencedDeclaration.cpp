#include "DereferencedDeclaration.h"

#include <algorithm>

#include "../code_generator/ValueEntry.h"
#include "../translation_unit/Context.h"

namespace ast {

DereferencedDeclaration::DereferencedDeclaration(std::unique_ptr<Declarator> declaration) :
        declaration { std::move(declaration) }
{
}

DereferencedDeclaration::~DereferencedDeclaration() {
}

void DereferencedDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) {
    declaration->accept(visitor);
}

std::string DereferencedDeclaration::getName() const {
    return declaration->getName();
}

int DereferencedDeclaration::getDereferenceCount() const {
    return declaration->getDereferenceCount() + 1;
}

translation_unit::Context DereferencedDeclaration::getContext() const {
    return declaration->getContext();
}

void DereferencedDeclaration::setHolder(code_generator::ValueEntry holder) {
    declaration->setHolder(holder);
}

code_generator::ValueEntry* DereferencedDeclaration::getHolder() const {
    return declaration->getHolder();
}

} /* namespace ast */
