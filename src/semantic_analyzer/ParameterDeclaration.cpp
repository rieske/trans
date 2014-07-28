#include "ParameterDeclaration.h"

#include <algorithm>
#include <vector>

#include "../code_generator/symbol_entry.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

const std::string ParameterDeclaration::ID { "<param_decl>" };

ParameterDeclaration::ParameterDeclaration(TerminalSymbol typeSpecifier, std::unique_ptr<Declaration> declaration, SymbolTable *st) :
		NonterminalNode(ID, { }, st, typeSpecifier.line),
		declaration { std::move(declaration) } {
	basic_type = typeSpecifier.value;
	extended_type = this->declaration->getType();
	name = this->declaration->getName();
	if (basic_type == "void" && extended_type == "") {
		semanticError("error: function parameter ‘" + name + "’ declared void\n");
	} else {
		place = new SymbolEntry(name, basic_type, extended_type, false, sourceLine);
		place->setParam();
	}
}

ParameterDeclaration::~ParameterDeclaration() {
}

string ParameterDeclaration::getBasicType() const {
	return basic_type;
}

string ParameterDeclaration::getExtendedType() const {
	return extended_type;
}

SymbolEntry *ParameterDeclaration::getPlace() const {
	return place;
}

void ParameterDeclaration::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
