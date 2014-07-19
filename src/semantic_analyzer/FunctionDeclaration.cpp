#include "FunctionDeclaration.h"

#include <cstdlib>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "Declaration.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

const std::string FunctionDeclaration::ID { "<func_decl>" };

FunctionDeclaration::FunctionDeclaration(TerminalSymbol typeSpecifier, Declaration* declaration, AbstractSyntaxTreeNode* block, SymbolTable *s_t, unsigned ln) :
		NonterminalNode(ID, { declaration, block }, s_t, ln) {
	name = declaration->getName();
	basic_type = typeSpecifier.value;
	extended_type = declaration->getType();
	code = block->getCode();
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

}
