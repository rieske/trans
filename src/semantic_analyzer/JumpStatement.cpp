#include "JumpStatement.h"

#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "Expression.h"

namespace semantic_analyzer {

const std::string JumpStatement::ID { "<jmp_stmt>" };

JumpStatement::JumpStatement(std::string jumpKeyword, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { }, st, ln) {
	if (jumpKeyword == "continue") {
		cerr << jumpKeyword << " not implemented yet!\n";
	} else if (jumpKeyword == "break") {
		cerr << jumpKeyword << " not implemented yet!\n";
	} else {
		cerr << jumpKeyword << " not implemented yet!\n";
	}
}

JumpStatement::JumpStatement(Expression* expression, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { expression }, st, ln) {
	code = expression->getCode();
	SymbolEntry *expr_val = expression->getPlace();
	code.push_back(new Quadruple(RETURN, expr_val, NULL, NULL));
	attr = expr_val->getBasicType() + expr_val->getExtendedType();
}

string JumpStatement::getAttr() const {
	return attr;
}

}
