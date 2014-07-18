#include "Term.h"

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

const std::string Term::ID { "<term>" };

Term::Term(TerminalSymbol term, SymbolTable *st, unsigned ln) :
		Expression(ID, { }, st, ln) {
	if (term.type == "id") {
		value = term.value;
		if ( NULL != (place = s_table->lookup(value))) {
			value = "lval";
			basic_type = place->getBasicType();
			extended_type = place->getExtendedType();
		} else {
			semanticError("symbol " + value + " is not defined\n");
		}
	} else if (term.type == "int_const") {
		value = "rval";
		basic_type = "int";
		place = s_table->newTemp(basic_type, "");
		code.push_back(new Quadruple(term.value, place));
	} else if (term.type == "float_const") {
		value = "rval";
		basic_type = "float";
		place = s_table->newTemp(basic_type, "");
		code.push_back(new Quadruple(term.value, place));
	} else if (term.type == "literal") {
		value = "rval";
		basic_type = "char";
		place = s_table->newTemp(basic_type, "");
		code.push_back(new Quadruple(term.value, place));
	} else if (term.type == "string") {
		value = term.value;
		basic_type = "char";
		extended_type = "a";
		place = s_table->newTemp(basic_type, extended_type);
		code.push_back(new Quadruple(term.value, place));
	} else {
		semanticError("bad term literal: " + term.value);
	}
}

Term::Term(Expression* expression, SymbolTable *st, unsigned ln) :
		Expression(ID, { expression }, st, ln) {
	saveExpressionAttributes(*expression);
}

}
