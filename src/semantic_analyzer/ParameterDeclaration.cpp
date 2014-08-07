#include "ParameterDeclaration.h"

#include <algorithm>

#include "../code_generator/symbol_entry.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"

namespace semantic_analyzer {

const std::string ParameterDeclaration::ID { "<param_decl>" };

ParameterDeclaration::ParameterDeclaration(TypeSpecifier type, std::unique_ptr<Declaration> declaration, SymbolTable *st) :
        NonterminalNode(st, declaration->getLineNumber()),
        type { type },
        declaration { std::move(declaration) } {
    auto basicType = type.getType();
    auto extended_type = this->declaration->getType();
    auto name = this->declaration->getName();
    if (basicType == BasicType::VOID && extended_type == "") {
        semanticError("error: function parameter ‘" + name + "’ declared void\n");
    } else {
        place = new SymbolEntry(name, basicType, extended_type, false, this->declaration->getLineNumber());
        place->setParam();
    }
}

ParameterDeclaration::~ParameterDeclaration() {
}

SymbolEntry *ParameterDeclaration::getPlace() const {
    return place;
}

void ParameterDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) const {
    visitor.visit(*this);
}

}
