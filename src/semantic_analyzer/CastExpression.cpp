#include "CastExpression.h"

#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "Pointer.h"

using parser::ParseTreeNode;

namespace semantic_analyzer {

const std::string CastExpression::ID { "<cast_expr>" };

CastExpression::CastExpression(ParseTreeNode* typeSpecifier, Expression* castExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { typeSpecifier, castExpression }, st, ln) {
	basic_type = typeSpecifier->getAttr();
	extended_type = "";
	SymbolEntry *arg = castExpression->getPlace();
	place = s_table->newTemp(basic_type, extended_type);
	code = castExpression->getCode();
	code.push_back(new Quadruple(ASSIGN, arg, NULL, place));
	value = "rval";
}

CastExpression::CastExpression(ParseTreeNode* typeSpecifier, Pointer* pointer, Expression* castExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { typeSpecifier, pointer, castExpression }, st, ln) {
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
	saveExpressionAttributes(*unaryExpression);
}

}
