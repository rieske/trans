#include "FunctionDefinition.h"

#include <algorithm>
#include <cstdlib>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"

namespace semantic_analyzer {

const std::string FunctionDefinition::ID { "<func_decl>" };

FunctionDefinition::FunctionDefinition(TerminalSymbol typeSpecifier, std::unique_ptr<Declaration> declaration,
		std::unique_ptr<AbstractSyntaxTreeNode> body, SymbolTable *s_t) :
		NonterminalNode(ID, { }, s_t, typeSpecifier.line),
		typeSpecifier { typeSpecifier },
		declaration { std::move(declaration) },
		body { std::move(body) } {
	name = this->declaration->getName();
	basic_type = typeSpecifier.value;
	extended_type = this->declaration->getType();
	code = this->body->getCode();
	SymbolEntry *entry = s_table->lookup(name);
	if (entry != NULL) {
		if ("" == entry->getBasicType() || basic_type == entry->getBasicType()) {
			entry->setBasicType(basic_type);
			entry->setExtendedType(extended_type);
		} else {
			semanticError(
					"recursive function " + name + " call used with wrong type! Can't convert " + entry->getBasicType() + " to "
							+ basic_type + "\n");
		}
		code.insert(code.begin(), new Quadruple(PROC, entry, NULL, NULL));
		code.insert(code.end(), new Quadruple(ENDPROC, entry, NULL, NULL));
	} else {
		semanticError("fatal error: could't lookup function entry to fill return values!\n");
		exit(1);
	}
}

void FunctionDefinition::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
