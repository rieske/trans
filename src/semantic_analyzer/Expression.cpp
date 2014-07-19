#include "Expression.h"

#include "../code_generator/symbol_table.h"

namespace semantic_analyzer {

const std::string Expression::ID { "<expr>" };

Expression::Expression(Expression* expression, Expression* assignmentExpression, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { expression, assignmentExpression }, st, ln) {
	place = nullptr;
	lval = nullptr;
	saveExpressionAttributes(*expression);
	vector<Quadruple *> more_code = assignmentExpression->getCode();
	code.insert(code.end(), more_code.begin(), more_code.end());
}

Expression::Expression(string label, vector<AbstractSyntaxTreeNode *> children, SymbolTable *st, unsigned ln) :
		NonterminalNode(label, children, st, ln) {
	// FIXME: is this required?
	place = nullptr;
	lval = nullptr;
}

string Expression::getBasicType() const {
	return basic_type;
}

string Expression::getExtendedType() const {
	return extended_type;
}

string Expression::getValue() const {
	return value;
}

SymbolEntry *Expression::getPlace() const {
	return place;
}

SymbolEntry *Expression::getLval() const {
	return lval;
}

void Expression::printCode() const {
	for (unsigned i = 0; i < code.size(); i++)
		code[i]->output(std::cout);
}

void Expression::saveExpressionAttributes(const Expression& expression) {
	value = expression.getValue();
	basic_type = expression.getBasicType();
	extended_type = expression.getExtendedType();
	code = expression.getCode();
	place = expression.getPlace();
	lval = expression.getLval();
}

Quadruple *Expression::backpatch(vector<Quadruple *> *bpList) {
	if (bpList->size()) {
		SymbolEntry *label = s_table->newLabel();
		for (unsigned i = 0; i < bpList->size(); i++) {
			bpList->at(i)->setArg1(label);
		}
		bpList->clear();
		return new Quadruple(LABEL, label, NULL, NULL);
	}
	return NULL;
}

Quadruple *Expression::backpatch(vector<Quadruple *> *bpList, Quadruple *q) {
	if (bpList->size()) {
		SymbolEntry *label = q->getArg1();
		for (unsigned i = 0; i < bpList->size(); i++)
			bpList->at(i)->setArg1(label);
		bpList->clear();
		return q;
	}
	return NULL;
}


}

