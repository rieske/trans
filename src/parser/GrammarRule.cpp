#include "GrammarRule.h"

#include "GrammarSymbol.h"

using std::shared_ptr;
using std::vector;

GrammarRule::GrammarRule(const shared_ptr<GrammarSymbol> nonterminal,
		const vector<shared_ptr<GrammarSymbol>> production, const int ruleId) :
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

std::ostream& operator<<(std::ostream& out, const GrammarRule& rule) {
	out << rule.id << ": " << rule.nonterminal << " -> ";
	for (auto& symbol : rule.production) {
		out << *symbol << " ";
	}
	out << "\n";
	return out;
}
