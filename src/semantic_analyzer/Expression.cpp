#include "Expression.h"

#include <cstdlib>
#include <iterator>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "../parser/GrammarSymbol.h"

namespace semantic_analyzer {

const std::string Expression::ID { "<expr>" };

Expression::Expression(Expression* expression, Expression* assignmentExpression, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { expression, assignmentExpression }, st, ln) {
	place = nullptr;
	lval = nullptr;
	getAttributes(0);
	vector<Quadruple *> more_code = assignmentExpression->getCode();
	code.insert(code.end(), more_code.begin(), more_code.end());
}

Expression::Expression(Expression* assignmentExpression, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { assignmentExpression }, st, ln) {
	place = nullptr;
	lval = nullptr;
	getAttributes(0);
}

Expression::Expression(string label, vector<ParseTreeNode *> children, SymbolTable *st, unsigned ln) :
		NonterminalNode(label, children, st, ln) {
	// FIXME: is this required?
	place = nullptr;
	lval = nullptr;
}

ostringstream &Expression::asXml(ostringstream &oss, unsigned depth) const {
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
	oss << " >" << endl;

	for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
		(*it)->asXml(oss, depth + 1);

	for (unsigned i = 0; i < depth; i++)
		oss << "    ";
	oss << "</" << elabel << ">" << endl;
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

void Expression::getAttributes(int subtree) {
	value = ((Expression *) subtrees.at(subtree))->getValue();
	basic_type = ((Expression *) subtrees.at(subtree))->getBasicType();
	extended_type = ((Expression *) subtrees.at(subtree))->getExtendedType();
	code = ((Expression *) subtrees.at(subtree))->getCode();
	place = ((Expression *) subtrees.at(subtree))->getPlace();
	lval = ((Expression *) subtrees.at(subtree))->getLval();
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
