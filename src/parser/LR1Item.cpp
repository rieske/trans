#include "LR1Item.h"

#include <bits/shared_ptr_base.h>
//#include <iostream>
#include <iterator>

#include "GrammarRule.h"
#include "GrammarSymbol.h"

using std::vector;
using std::shared_ptr;

LR1Item::LR1Item(shared_ptr<GrammarRule> rule, shared_ptr<GrammarSymbol> lookahead) :
		rule { rule } {
	lookaheads.push_back(lookahead);
}

LR1Item::~LR1Item() {
}

void LR1Item::advance() {
	++visitedOffset;
}

bool LR1Item::operator==(const LR1Item& rhs) const {
	return (this->rule == rhs.rule) && (this->visitedOffset == rhs.visitedOffset) && (this->lookaheads == rhs.lookaheads);
}

bool LR1Item::operator!=(const LR1Item& rhs) const {
	return !(*this == rhs);
}

void LR1Item::print() const {
	log(std::cerr);
}

void LR1Item::log(std::ostream &out) const {
	out << "[ " << *rule->getNonterminal() << " -> ";
	const auto& seenSymbols = getSeen();
	for (const auto& seenSymbol : seenSymbols) {
		out << seenSymbol << " ";
	}
	out << ". ";
	const auto& expectedSymbols = getExpected();
	for (auto& expectedSymbol : expectedSymbols) {
		out << *expectedSymbol << " ";
	}
	out << ", ";
	for (unsigned i = 0; i < lookaheads.size(); i++) {
		out << lookaheads.at(i);
		if (i != lookaheads.size() - 1)
			out << "/";
	}
	out << " ]\n";
}

bool LR1Item::coresAreEqual(const LR1Item& that) const {
	return (this->rule == that.rule) && (this->visitedOffset == that.visitedOffset);
}

void LR1Item::mergeLookaheads(const LR1Item& item) {
	if (lookaheads.empty()) {
		for (unsigned i = 0; i < item.lookaheads.size(); i++)
			lookaheads.push_back(item.lookaheads.at(i));
		return;
	}
	for (unsigned i = 0; i < item.lookaheads.size(); i++) {
		bool insert = true;
		unsigned j = 0;
		for (; j < this->lookaheads.size(); j++) {
			if (item.lookaheads.at(i) == this->lookaheads.at(j)) {
				insert = false;
				break;
			}
		}
		if (insert)
			this->lookaheads.push_back(item.lookaheads.at(i));
	}
}

shared_ptr<GrammarSymbol> LR1Item::getDefiningSymbol() const {
	return rule->getNonterminal();
}

vector<shared_ptr<GrammarSymbol>> LR1Item::getSeen() const {
	const auto& ruleProduction = rule->getProduction();
	return vector<shared_ptr<GrammarSymbol>> { ruleProduction.begin(), ruleProduction.begin() + visitedOffset };
}

vector<shared_ptr<GrammarSymbol>> LR1Item::getExpected() const {
	const auto& ruleProduction = rule->getProduction();
	return vector<shared_ptr<GrammarSymbol>> { ruleProduction.begin() + visitedOffset, ruleProduction.end() };
}

vector<shared_ptr<GrammarSymbol>> LR1Item::getLookaheads() const {
	return lookaheads;
}

