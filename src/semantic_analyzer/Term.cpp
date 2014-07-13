#include "Term.h"

#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"

namespace semantic_analyzer {

const std::string Term::ID { "<term>" };

Term::Term(ParseTreeNode* termLiteral, std::string termType, SymbolTable *st, unsigned ln) :
		Expression(ID, { termLiteral }, st, ln) {
	if (termType == "id") {
		value = termLiteral->getAttr();
		if ( NULL != (place = s_table->lookup(value))) {
			value = "lval";
			basic_type = place->getBasicType();
			extended_type = place->getExtendedType();
		} else {
			semanticError("symbol " + value + " is not defined\n");
		}
	} else if (termType == "int_const") {
		value = "rval";
		basic_type = "int";
		place = s_table->newTemp(basic_type, "");
		code.push_back(new Quadruple(termLiteral->getAttr(), place));
	} else if (termType == "float_const") {
		value = "rval";
		basic_type = "float";
		place = s_table->newTemp(basic_type, "");
		code.push_back(new Quadruple(termLiteral->getAttr(), place));
	} else if (termType == "literal") {
		value = "rval";
		basic_type = "char";
		place = s_table->newTemp(basic_type, "");
		code.push_back(new Quadruple(termLiteral->getAttr(), place));
	} else if (termType == "string") {
		value = termLiteral->getAttr();
		basic_type = "char";
		extended_type = "a";
		place = s_table->newTemp(basic_type, extended_type);
		code.push_back(new Quadruple(termLiteral->getAttr(), place));
	} else {
		semanticError("bad term literal: " + termLiteral->getAttr());
	}
}

Term::Term(ParseTreeNode* openParenthesis, Expression* expression, ParseTreeNode* closeParenthesis, SymbolTable *st, unsigned ln) :
		Expression(ID, { openParenthesis, expression, closeParenthesis }, st, ln) {
	getAttributes(1);
}

}
