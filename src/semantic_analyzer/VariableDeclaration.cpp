#include "VariableDeclaration.h"

#include <algorithm>
#include <sstream>
#include <vector>

#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"
#include "DeclarationList.h"

namespace semantic_analyzer {

const std::string VariableDeclaration::ID { "<var_decl>" };

VariableDeclaration::VariableDeclaration(TerminalSymbol typeSpecifier, std::unique_ptr<DeclarationList> declarationList, SymbolTable *st) :
		NonterminalNode(ID, { }, st, typeSpecifier.line),
		typeSpecifier { typeSpecifier },
		declarationList { std::move(declarationList) } {
	basic_type = typeSpecifier.value;
	int errLine;
	for (const auto& declaration : this->declarationList->getDeclarations()) {
		if (basic_type == "void" && declaration->getType() == "") {
			semanticError("error: variable or field ‘" + declaration->getName() + "’ declared void\n");
		} else if (0 != (errLine = s_table->insert(declaration->getName(), basic_type, declaration->getType(), sourceLine))) {
			std::ostringstream errorDescription;
			errorDescription << "symbol " << declaration->getName() << " declaration conflicts with previous declaration on line "
					<< errLine << "\n";
			semanticError(errorDescription.str());
		}
	}
}

void VariableDeclaration::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
