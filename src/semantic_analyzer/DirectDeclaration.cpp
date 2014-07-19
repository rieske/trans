#include "DirectDeclaration.h"

#include <cstdlib>
#include <iterator>
#include <sstream>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "ParameterList.h"
#include "TerminalSymbol.h"
#include "Declaration.h"
#include "DirectDeclaration.h"
#include "LogicalOrExpression.h"

namespace semantic_analyzer {

const std::string DirectDeclaration::ID { "<dir_decl>" };

DirectDeclaration::DirectDeclaration(Declaration* declaration, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { declaration }, st, ln) {
	//if (reduction == "'(' <decl> ')'") {  // XXX: čia žiūrėti reiks

	//}
}

DirectDeclaration::DirectDeclaration(TerminalSymbol identifier, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { }, st, ln) {
	name = identifier.value;
}

DirectDeclaration::DirectDeclaration(DirectDeclaration* directDeclaration, ParameterList* parameterList, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { directDeclaration, parameterList }, st, ln) {
	name = directDeclaration->getName();
	type = "f";
	params = parameterList->getParams();
	int errLine;
	if (0 != (errLine = s_table->insert(name, "", type, sourceLine))) {
		std::ostringstream errorDescription;
		errorDescription << "symbol " << name << " declaration conflicts with previous declaration on line " << errLine << "\n";
		semanticError(errorDescription.str());
	} else {
		SymbolEntry *place = s_table->lookup(name);
		for (unsigned i = 0; i < params.size(); i++) {
			if (params[i]->getPlace() == NULL) {
				std::cerr << "params.place == nullptr\n";
				exit(2);
			}
			SymbolEntry *entry = params[i]->getPlace();
			place->setParam(entry);
		}
	}
}

// Array declaration
DirectDeclaration::DirectDeclaration(DirectDeclaration* directDeclaration, LogicalOrExpression* logicalExpression, SymbolTable *st,
		unsigned ln) :
		NonterminalNode(ID, { directDeclaration, logicalExpression }, st, ln) {
	// FIXME: not implemented
	name = directDeclaration->getName();
	type = "a";
}

DirectDeclaration::DirectDeclaration(DirectDeclaration* directDeclaration, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { directDeclaration }, st, ln) {
	name = directDeclaration->getName();
	type = "f";
	int errLine;
	if (0 != (errLine = s_table->insert(name, "", type, sourceLine))) {
		std::ostringstream errorDescription;
		errorDescription << "symbol " << name << " declaration conflicts with previous declaration on line " << errLine << "\n";
		semanticError(errorDescription.str());
	}
}

string DirectDeclaration::getName() const {
	return name;
}

string DirectDeclaration::getType() const {
	return type;
}

vector<ParameterDeclaration *> DirectDeclaration::getParams() const {
	return params;
}

}
