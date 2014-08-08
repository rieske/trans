#include "VariableDeclaration.h"

#include <algorithm>
#include <sstream>

#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"
#include "Declaration.h"
#include "DeclarationList.h"

namespace semantic_analyzer {

const std::string VariableDeclaration::ID { "<var_decl>" };

VariableDeclaration::VariableDeclaration(TypeSpecifier type, std::unique_ptr<DeclarationList> declarationList, SymbolTable *st) :
		NonterminalNode(st, 0),
		declaredType { type },
		declaredVariables { std::move(declarationList) } {
	BasicType basicType = type.getType();
	for (const auto& declaration : this->declaredVariables->getDeclarations()) {
		int errLine;
		if (basicType == BasicType::VOID && declaration->getType() == "") {
			semanticError("error: variable or field ‘" + declaration->getName() + "’ declared void\n");
		} else if (0 != (errLine = s_table->insert(declaration->getName(), basicType, declaration->getType(), sourceLine))) {
			std::ostringstream errorDescription;
			errorDescription << "symbol " << declaration->getName() << " declaration conflicts with previous declaration on line "
					<< errLine << "\n";
			semanticError(errorDescription.str());
		}
	}
}

void VariableDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

}
