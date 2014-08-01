#include "FunctionDeclaration.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "ParameterList.h"

namespace semantic_analyzer {

FunctionDeclaration::FunctionDeclaration(std::unique_ptr<Declaration> directDeclaration, SymbolTable *st, unsigned ln) :
		FunctionDeclaration(std::move(directDeclaration), std::unique_ptr<ParameterList> { new ParameterList() }, st, ln) {
}

FunctionDeclaration::FunctionDeclaration(std::unique_ptr<Declaration> directDeclaration, std::unique_ptr<ParameterList> parameterList,
		SymbolTable *st, unsigned ln) :
		Declaration(st, ln),
		declaration { std::move(directDeclaration) },
		parameterList { std::move(parameterList) } {
	name = this->declaration->getName();
	type = "f";
	for (const auto& param : this->parameterList->getDeclaredParameters()) {
		params.push_back(param.get());
	}
	int errLine;
	if (0 != (errLine = s_table->insert(name, "", type, sourceLine))) {
		std::ostringstream errorDescription;
		errorDescription << "symbol " << name << " declaration conflicts with previous declaration on line " << errLine << "\n";
		semanticError(errorDescription.str());
	} else {
		SymbolEntry *place = s_table->lookup(name);
		for (unsigned i = 0; i < params.size(); i++) {
			if (params[i]->getPlace() == NULL) {
				semanticError("params.place == nullptr");
			}
			SymbolEntry *entry = params[i]->getPlace();
			place->setParam(entry);
		}
	}
}

FunctionDeclaration::~FunctionDeclaration() {
}

const vector<ParameterDeclaration *> FunctionDeclaration::getParams() const {
	return params;
}

void FunctionDeclaration::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
