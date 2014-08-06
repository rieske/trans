#include "ParameterDeclaration.h"

#include <algorithm>

#include "../code_generator/symbol_entry.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"

namespace semantic_analyzer {

const std::string ParameterDeclaration::ID { "<param_decl>" };

ParameterDeclaration::ParameterDeclaration(TypeSpecifier type, std::unique_ptr<Declaration> declaration, SymbolTable *st) :
        NonterminalNode(st, 0),
        type { type },
        declaration { std::move(declaration) } {
    basicType = type.getType();
    extended_type = this->declaration->getType();
    name = this->declaration->getName();
    if (basicType == BasicType::VOID && extended_type == "") {
        semanticError("error: function parameter ‘" + name + "’ declared void\n");
    } else {
        place = new SymbolEntry(name, basicType, extended_type, false, sourceLine);
        place->setParam();
    }
}

ParameterDeclaration::~ParameterDeclaration() {
}

BasicType ParameterDeclaration::getBasicType() const {
    return basicType;
}

string ParameterDeclaration::getExtendedType() const {
    return extended_type;
}

SymbolEntry *ParameterDeclaration::getPlace() const {
    return place;
}

void ParameterDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) const {
    visitor.visit(*this);
}

}
