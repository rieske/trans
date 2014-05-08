#include "GrammarRule.h"

#include <sstream>
#include <string>

#include "GrammarSymbol.h"

using std::cerr;
using std::endl;

GrammarRule::GrammarRule(std::shared_ptr<GrammarSymbol> left, int ruleId) :
		left { left },
		right { new vector<std::shared_ptr<GrammarSymbol>> },
		id { ruleId } {
}

GrammarRule::~GrammarRule() {
	delete right;
}

std::shared_ptr<GrammarSymbol> GrammarRule::getLeft() const {
	return left;
}

unsigned GrammarRule::getId() const {
	return id;
}

void GrammarRule::print() const {
	cerr << id << ": " << left << " -> ";
	for (unsigned i = 0; i != right->size(); i++)
		cerr << right->at(i) << " ";
	cerr << endl;
}

void GrammarRule::printAddr() const {
	cerr << left << " -> ";
	for (unsigned i = 0; i != right->size(); i++)
		cerr << right->at(i) << " ";
	cerr << endl;
}

void GrammarRule::log(ostream &out) const {
	out << id << ": " << left << " : ";
	for (unsigned i = 0; i != right->size(); i++)
		out << right->at(i) << " ";
	out << endl;
}

vector<std::shared_ptr<GrammarSymbol>> *GrammarRule::getRight() const {
	return right;
}

void GrammarRule::addRight(std::shared_ptr<GrammarSymbol> r) {
	right->push_back(r);
}

string GrammarRule::rightStr() const {
	string ret = "";
	for (auto& rightSymbol : *right) {
		ret += rightSymbol->getName();
		ret += " ";
	}
	return ret.substr(0, ret.size() - 1);
}
