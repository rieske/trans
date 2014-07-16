#include "Expression.h"

#include <cstdlib>
#include <iterator>
#include <sstream>


#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "../parser/GrammarSymbol.h"

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

Expression::Expression(Expression* assignmentExpression, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { assignmentExpression }, st, ln) {
	place = nullptr;
	lval = nullptr;
	saveExpressionAttributes(*assignmentExpression);
}

Expression::Expression(string label, vector<ParseTreeNode *> children, SymbolTable *st, unsigned ln) :
		NonterminalNode(label, children, st, ln) {
	// FIXME: is this required?
	place = nullptr;
	lval = nullptr;
}

std::ostringstream &Expression::asXml(std::ostringstream &oss, unsigned depth) const {
	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	string elabel = xmlEncode(label);
	oss << "<" << elabel << " basic_type=\"" << xmlEncode(basic_type) << "\"";
	if (extended_type != "")
		oss << " extended_type=\"" << extended_type << "\"";
	if (value != "")
		oss << " value=\"" << xmlEncode(value) << "\"";
	else if (place != NULL)
		oss << " place=\"" << place->getName() << "\"";
	oss << " code_size=\"" << code.size() << "\"";
	oss << " >\n";

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">\n";
	return oss;
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
