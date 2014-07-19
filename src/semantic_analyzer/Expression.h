#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "NonterminalNode.h"

class SymbolTable;

namespace semantic_analyzer {

class Expression: public NonterminalNode {
public:
	Expression(Expression* expression, Expression* assignmentExpression, SymbolTable *st, unsigned ln);

	virtual string getBasicType() const;
	virtual string getExtendedType() const;
	virtual string getValue() const;

	virtual SymbolEntry *getPlace() const;
	virtual SymbolEntry *getLval() const;

	void printCode() const;

	static const std::string ID;

protected:
	Expression(std::string label, vector<AbstractSyntaxTreeNode *> children, SymbolTable *st, unsigned ln);

	void saveExpressionAttributes(const Expression& expression);

	Quadruple *backpatch(vector<Quadruple *> *bpList);
	Quadruple *backpatch(vector<Quadruple *> *bpList, Quadruple* q);

	string basic_type;
	string extended_type;
	string value;

	SymbolEntry *place;
	SymbolEntry *lval;
};

}

#endif // _EXPR_NODE_H_
