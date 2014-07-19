#include "ParameterDeclaration.h"

#include <vector>

#include "../code_generator/symbol_entry.h"
#include "Declaration.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

const std::string ParameterDeclaration::ID { "<param_decl>" };

ParameterDeclaration::ParameterDeclaration(TerminalSymbol typeSpecifier, Declaration* declaration, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { declaration }, st, ln) {
	basic_type = typeSpecifier.value;
	extended_type = declaration->getType();
	name = declaration->getName();
	if (basic_type == "void" && extended_type == "") {
		semanticError("error: function parameter ‘" + name + "’ declared void\n");
	} else {
		place = new SymbolEntry(name, basic_type, extended_type, false, sourceLine);
		place->setParam();
	}
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

}
