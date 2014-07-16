#include "FunctionDeclaration.h"

#include <cstdlib>
#include <iterator>
#include <vector>
#include <sstream>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "Declaration.h"

namespace semantic_analyzer {

const std::string FunctionDeclaration::ID { "<func_decl>" };

FunctionDeclaration::FunctionDeclaration(ParseTreeNode* typeSpecifier, Declaration* declaration, ParseTreeNode* block, SymbolTable *s_t, unsigned ln) :
		NonterminalNode(ID, { typeSpecifier, declaration, block }, s_t, ln) {
	name = declaration->getName();
	basic_type = typeSpecifier->getAttr();
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

std::ostringstream &FunctionDeclaration::asXml(std::ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel << " basic_type=\"" << xmlEncode(basic_type) << "\"";
	if (extended_type != "")
		oss << " extended_type=\"" << xmlEncode(extended_type) << "\"";
	oss << " >\n";

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">\n";
	return oss;
}

}
