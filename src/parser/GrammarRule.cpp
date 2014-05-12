#include "GrammarRule.h"

#include <sstream>
#include <string>

#include "GrammarSymbol.h"

using std::cerr;
using std::endl;
using std::shared_ptr;
using std::vector;

GrammarRule::GrammarRule(const shared_ptr<GrammarSymbol> nonterminal, const vector<shared_ptr<GrammarSymbol>> production, const int ruleId) :
		nonterminal { nonterminal },
		production { production },
		id { ruleId } {
}

GrammarRule::~GrammarRule() {
}

shared_ptr<GrammarSymbol> GrammarRule::getNonterminal() const {
	return nonterminal;
}

int GrammarRule::getId() const {
	return id;
}

void GrammarRule::print() const {
	cerr << id << ": " << nonterminal << " -> ";
	for (auto& symbol : production) {
		cerr << *symbol << " ";
	}
	cerr << endl;
}

void GrammarRule::log(std::ostream &out) const {
	out << id << ": " << nonterminal << " -> ";
	for (auto& symbol : production) {
		out << *symbol << " ";
	}
	out << endl;
}

vector<shared_ptr<GrammarSymbol>> GrammarRule::getProduction() const {
	return production;
}

std::string GrammarRule::rightStr() const {
	std::string ret = "";
	for (auto& symbol : production) {
		ret += symbol->getName();
		ret += " ";
	}
	return ret.substr(0, ret.size() - 1);
}
