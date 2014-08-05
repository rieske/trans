#include "Expression.h"

#include "../code_generator/symbol_table.h"

namespace semantic_analyzer {

const std::string Expression::ID { "<expr>" };

Expression::Expression(SymbolTable *st, unsigned ln) :
		NonterminalNode(st, ln) {
}

BasicType Expression::getBasicType() const {
	return basicType;
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

void Expression::saveExpressionAttributes(const Expression& expression) {
	value = expression.getValue();
	basicType = expression.getBasicType();
	extended_type = expression.getExtendedType();
	code = expression.getCode();
	place = expression.getPlace();
	lval = expression.getLval();
}

void Expression::backpatch() {
	if (!backpatchList.empty()) {
		SymbolEntry *label = s_table->newLabel();
		for (unsigned i = 0; i < backpatchList.size(); i++) {
			backpatchList.at(i)->setArg1(label);
		}
		backpatchList.clear();
		code.push_back(new Quadruple(LABEL, label, NULL, NULL));
	}
}

vector<Quadruple *> Expression::getBackpatchList() const {
	return backpatchList;
}

}

