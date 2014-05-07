#include "rule.h"

#include <sstream>
#include <string>

#include "GrammarSymbol.h"

using std::cerr;
using std::endl;

Rule::Rule(std::shared_ptr<GrammarSymbol> left, int ruleId) :
		left { left },
		right { new vector<std::shared_ptr<GrammarSymbol>> },
		id { ruleId } {
}

Rule::~Rule() {
	delete right;
}

std::shared_ptr<GrammarSymbol> Rule::getLeft() const {
	return left;
}

unsigned Rule::getId() const {
	return id;
}

void Rule::print() const {
	cerr << id << ": " << left << " -> ";
	for (unsigned i = 0; i != right->size(); i++)
		cerr << right->at(i) << " ";
	cerr << endl;
}

void Rule::printAddr() const {
	cerr << left << " -> ";
	for (unsigned i = 0; i != right->size(); i++)
		cerr << right->at(i) << " ";
	cerr << endl;
}

void Rule::log(ostream &out) const {
	out << id << ": " << left << " : ";
	for (unsigned i = 0; i != right->size(); i++)
		out << right->at(i) << " ";
	out << endl;
}

vector<std::shared_ptr<GrammarSymbol>> *Rule::getRight() const {
	return right;
}

void Rule::addRight(std::shared_ptr<GrammarSymbol> r) {
	right->push_back(r);
}

string Rule::rightStr() const {
	string ret = "";
	for (auto& rightSymbol : *right) {
		ret += rightSymbol->getName();
		ret += " ";
	}
	return ret.substr(0, ret.size() - 1);
}
