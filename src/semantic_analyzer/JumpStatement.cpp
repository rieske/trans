#include "JumpStatement.h"

#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "Expression.h"

namespace semantic_analyzer {

const std::string JumpStatement::ID { "<jmp_stmt>" };

JumpStatement::JumpStatement(ParseTreeNode* jumpKeyword, ParseTreeNode* semicolon, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { jumpKeyword, semicolon }, st, ln) {
	if (jumpKeyword->getAttr() == "'continue'") {
		cerr << jumpKeyword->getAttr() << " not implemented yet!\n";
	} else if (jumpKeyword->getAttr() == "'break'") {
		cerr << jumpKeyword->getAttr() << " not implemented yet!\n";
	} else {
		cerr << jumpKeyword->getAttr() << " not implemented yet!\n";
	}
}

JumpStatement::JumpStatement(ParseTreeNode* returnKeyword, Expression* expression, ParseTreeNode* semicolon, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { returnKeyword, expression, semicolon }, st, ln) {
	code = expression->getCode();
	SymbolEntry *expr_val = expression->getPlace();
	code.push_back(new Quadruple(RETURN, expr_val, NULL, NULL));
	attr = expr_val->getBasicType() + expr_val->getExtendedType();
}

string JumpStatement::getAttr() const {
	return attr;
}

}
