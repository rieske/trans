#include "ParameterDeclaration.h"

#include <iterator>
#include <vector>
#include <sstream>

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

std::ostringstream &ParameterDeclaration::asXml(std::ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel << " basic_type=\"" << xmlEncode(basic_type) << "\"";
	if (extended_type != "")
		oss << " extended_type=\"" << xmlEncode(extended_type) << "\"";
	((Declaration *) subtrees[1])->output_attr(oss, 0);
	oss << " >\n";

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">\n";
	return oss;
}

void ParameterDeclaration::output_attr(std::ostringstream &oss, unsigned nr) const {
	oss << " name" << nr << "=\"" << name << "\"";
	if (basic_type != "")
		oss << " basic_type" << nr << "=\"" << basic_type << "\"";
	if (extended_type != "")
		oss << " extended_type" << nr << "=\"" << extended_type << "\"";
}

SymbolEntry *ParameterDeclaration::getPlace() const {
	return place;
}

}
