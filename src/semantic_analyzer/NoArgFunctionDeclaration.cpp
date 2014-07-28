#include "NoArgFunctionDeclaration.h"

#include <algorithm>
#include <sstream>
#include <string>

#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

NoArgFunctionDeclaration::NoArgFunctionDeclaration(std::unique_ptr<Declaration> declaration, SymbolTable *st, unsigned ln) :
		Declaration(st, ln),
		declaration { std::move(declaration) } {
	name = this->declaration->getName();
	type = "f";
	int errLine;
	if (0 != (errLine = s_table->insert(name, "", type, sourceLine))) {
		std::ostringstream errorDescription;
		errorDescription << "symbol " << name << " declaration conflicts with previous declaration on line " << errLine << "\n";
		semanticError(errorDescription.str());
	}
}

NoArgFunctionDeclaration::~NoArgFunctionDeclaration() {
}

void NoArgFunctionDeclaration::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
