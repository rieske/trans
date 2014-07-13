#include "CastExpression.h"

#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "Pointer.h"

namespace semantic_analyzer {

const std::string CastExpression::ID { "<cast_expr>" };

CastExpression::CastExpression(ParseTreeNode* openParenthesis, ParseTreeNode* typeSpecifier, ParseTreeNode* closeParenthesis,
		Expression* castExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { openParenthesis, typeSpecifier, closeParenthesis, castExpression }, st, ln) {
	basic_type = typeSpecifier->getAttr();
	extended_type = "";
	SymbolEntry *arg = castExpression->getPlace();
	place = s_table->newTemp(basic_type, extended_type);
	code = castExpression->getCode();
	code.push_back(new Quadruple(ASSIGN, arg, NULL, place));
	value = "rval";
}

CastExpression::CastExpression(ParseTreeNode* openParenthesis, ParseTreeNode* typeSpecifier, Pointer* pointer,
		ParseTreeNode* closeParenthesis, Expression* castExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { openParenthesis, typeSpecifier, pointer, closeParenthesis, castExpression }, st, ln) {
	basic_type = typeSpecifier->getAttr();
	extended_type = pointer->getType();
	SymbolEntry *arg = castExpression->getPlace();
	place = s_table->newTemp(basic_type, extended_type);
	code = castExpression->getCode();
	code.push_back(new Quadruple(ASSIGN, arg, NULL, place));
	value = "rval";
}

CastExpression::CastExpression(Expression* unaryExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { unaryExpression }, st, ln) {
	getAttributes(0);
}

}
