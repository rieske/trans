#include "AssignmentExpression.h"

#include <cstdlib>
#include <iterator>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "../parser/TerminalNode.h"

namespace semantic_analyzer {

const std::string AssignmentExpression::ID = "<a_expr>";

AssignmentExpression::AssignmentExpression(Expression* unaryExpression, std::string assignmentOperator, Expression* assignmentExpression,
		SymbolTable *st, unsigned ln) :
		LogicalOrExpression(ID, { unaryExpression, assignmentExpression }, st, ln) {
	saveExpressionAttributes(*unaryExpression);
	place = unaryExpression->getPlace();
	value = unaryExpression->getValue();
	bool deref = false;
	for (vector<Quadruple *>::iterator it = code.begin(); it != code.end(); it++) {
		if ((*it)->getOp() == DEREF) {
			deref = true;
		}
	}
	if (value != "lval") {
		if (lval == NULL) {
			semanticError("lvalue required on the left side of assignment\n");
		}
		place = lval;
	}
	SymbolEntry *arg_place = assignmentExpression->getPlace();
	vector<Quadruple *> a_code = assignmentExpression->getCode();
	code.insert(code.begin(), a_code.begin(), a_code.end());
	string check = s_table->typeCheck(arg_place, place);
	if (check != "ok") {
		semanticError(check);
	} else {
		if (assignmentOperator == "+=")
			code.push_back(new Quadruple(ADD, place, arg_place, place));
		else if (assignmentOperator == "-=")
			code.push_back(new Quadruple(SUB, place, arg_place, place));
		else if (assignmentOperator == "*=")
			code.push_back(new Quadruple(MUL, place, arg_place, place));
		else if (assignmentOperator == "/=")
			code.push_back(new Quadruple(DIV, place, arg_place, place));
		else if (assignmentOperator == "%=")
			code.push_back(new Quadruple(MOD, place, arg_place, place));
		else if (assignmentOperator == "&=")
			code.push_back(new Quadruple(AND, place, arg_place, place));
		else if (assignmentOperator == "^=")
			code.push_back(new Quadruple(XOR, place, arg_place, place));
		else if (assignmentOperator == "|=")
			code.push_back(new Quadruple(OR, place, arg_place, place));
		else if (assignmentOperator == "<<=")
			code.push_back(new Quadruple(SHL, place, arg_place, place));
		else if (assignmentOperator == ">>=")
			code.push_back(new Quadruple(SHR, place, arg_place, place));
		else if (assignmentOperator == "=") {
			if (deref) {
				Quadruple *quad = new Quadruple(DEREF_LVAL, arg_place, NULL, code.back()->getArg1());
				code.pop_back();
				code.push_back(quad);
			} else {
				code.push_back(new Quadruple(ASSIGN, arg_place, NULL, place));
			}
		} else {
			semanticError("unidentified assignment operator!\n");
			exit(1);
		}
	}
}

AssignmentExpression::AssignmentExpression(LogicalOrExpression* logExpr, SymbolTable *st, unsigned ln) :
		LogicalOrExpression(ID, { logExpr }, st, ln) {
	saveExpressionAttributes(*logExpr);
	backpatchList = logExpr->getBPList();
	Quadruple *q = backpatch(&backpatchList);
	if (q != NULL) {
		code.push_back(q);
	}
}

void AssignmentExpression::output_attr(ostringstream &oss, unsigned nr) const {
	if (basic_type != "")
		oss << " basic_type" << nr << "=\"" << basic_type << "\"";
	if (extended_type != "")
		oss << " extended_type" << nr << "=\"" << extended_type << "\"";
}

}
